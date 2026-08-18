module;

#include "proto/messenger.grpc.pb.h"

#include <CLI/CLI.hpp>

#include <grpcpp/grpcpp.h>

#include <atomic>
#include <condition_variable>
#include <cstdlib>
#include <exception>
#include <format>
#include <future>
#include <iostream>
#include <map>
#include <mutex>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

module will.client.willclient;

import will.client.clientconfigvalidator;
import will.client.devicetokenstore;


namespace will {


WillClient::WillClient() = default;


WillClient::WillClient(ClientConfig config)
    : config_(ClientConfigValidator::accept(std::move(config)))
{}


const ClientConfig& WillClient::config() const noexcept
{
    return config_;
}


void WillClient::set_inbound_handler(std::function<void(const v1::ServerEvent&)> handler)
{
    std::lock_guard lock(handler_mutex_);
    inbound_handler_ = std::move(handler);
}


void WillClient::set_closed_handler(std::function<void()> handler)
{
    std::lock_guard lock(handler_mutex_);
    closed_handler_ = std::move(handler);
}


void WillClient::dispatch_inbound(const v1::ServerEvent& event)
{
    std::function<void(const v1::ServerEvent&)> handler;
    {
        std::lock_guard lock(handler_mutex_);
        handler = inbound_handler_;
    }

    if (handler)
        handler(event);
}


void WillClient::dispatch_closed()
{
    if (pending_auth_) {
        try {
            pending_auth_->set_exception(
                std::make_exception_ptr(std::runtime_error("Will protocol: unexpected end of stream")));
        } catch (const std::future_error&) {
        }
        pending_auth_.reset();
    }

    std::function<void()> handler;
    {
        std::lock_guard lock(handler_mutex_);
        handler = closed_handler_;
    }

    if (handler)
        handler();
}


v1::ServerEvent WillClient::wait_for_auth_response()
{
    pending_auth_ = std::make_shared<std::promise<v1::ServerEvent>>();
    return pending_auth_->get_future().get();
}


bool WillClient::write_event(const v1::ClientEvent& event) const
{
    std::lock_guard lock(write_mutex_);

    if (!stream_ || closed_.load())
        return false;

    return stream_->Write(event);
}


void WillClient::reader_loop()
{
    v1::ServerEvent event;
    while (stream_ && stream_->Read(&event)) {
        if (pending_auth_) {
            try {
                pending_auth_->set_value(event);
            } catch (const std::future_error&) {
            }
            pending_auth_.reset();
            continue;
        }

        dispatch_inbound(event);
    }

    closed_.store(true);
    dispatch_closed();
}


void WillClient::connect()
{
    if (stream_)
        throw std::logic_error("WillClient: already connected");

    const std::string target = config_.host + ":" + std::to_string(config_.port);
    channel_ = grpc::CreateChannel(target, grpc::InsecureChannelCredentials());
    stub_ = v1::Messenger::NewStub(channel_);
    context_ = std::make_unique<grpc::ClientContext>();
    stream_ = stub_->Session(context_.get());
    if (!stream_)
        throw std::runtime_error("WillClient: failed to open Session stream");

    reader_thread_ = decltype(reader_thread_)([this] { reader_loop(); });
}


void WillClient::authenticate_device(const std::string_view device_token)
{
    if (!stream_)
        throw std::logic_error("WillClient: not connected");

    if (authenticated_)
        throw std::logic_error("WillClient: already authenticated");

    v1::ClientEvent event;
    event.mutable_bind_token()->set_token(std::string(device_token));
    if (!write_event(event))
        throw std::runtime_error("Will protocol: failed to send BindToken");

    const v1::ServerEvent response = wait_for_auth_response();
    if (response.has_auth_required())
        throw std::runtime_error("Will protocol: device authentication failed");

    if (!response.has_auth_ok())
        throw std::runtime_error("Will protocol: expected AuthOk");

    authenticated_ = true;
}


void WillClient::send(const std::string_view utf8_chat_body) const
{
    if (!authenticated_)
        throw std::logic_error("WillClient: not authenticated");

    v1::ClientEvent event;
    event.mutable_chat()->set_body(std::string(utf8_chat_body));

    if (!write_event(event))
        throw std::runtime_error("Will protocol: failed to send chat message");
}


bool WillClient::requestHistory(const std::uint32_t limit) const
{
    if (limit == 0)
        return false;

    if (!authenticated_)
        throw std::logic_error("WillClient: not authenticated");

    v1::ClientEvent event;
    event.mutable_history_request()->set_limit(limit);
    if (!write_event(event))
        throw std::runtime_error("Will protocol: failed to send HistoryRequest");
    return true;
}


void WillClient::shutdown() const
{
    if (shutdown_done_)
        return;

    shutdown_done_ = true;
    closed_.store(true);

    {
        std::lock_guard lock(write_mutex_);
        if (stream_)
            stream_->WritesDone();
    }

    if (context_)
        context_->TryCancel();

    if (reader_thread_.joinable())
        reader_thread_.join();

    stream_.reset();
    stub_.reset();
    context_.reset();
    channel_.reset();
}


namespace {


bool is_post_auth_server_event(const v1::ServerEvent& event) noexcept
{
    switch (event.event_case()) {
    case v1::ServerEvent::kReceiptAck:
    case v1::ServerEvent::kAuthRequired:
    case v1::ServerEvent::kChat:
    case v1::ServerEvent::kHistoryItem:
    case v1::ServerEvent::kHistoryEnd:
        return true;
    default:
        return false;
    }
}


void print_history_item(ConsoleUi& ui, const v1::HistoryItem& item)
{
    if (item.is_mine())
        ui.print_mine(item.body(), true);
    else
        ui.print_peer(item.name(), item.body(), true);
}


class LoadingHistoryMessageHandler final {
public:
    explicit LoadingHistoryMessageHandler(ConsoleUi& ui)
        : ui_(ui)
    {}

    bool history_finished() const noexcept { return history_finished_; }

    void on(const v1::ServerEvent& event)
    {
        switch (event.event_case()) {
        case v1::ServerEvent::kHistoryItem:
            print_history_item(ui_, event.history_item());
            return;
        case v1::ServerEvent::kHistoryEnd:
            history_finished_ = true;
            return;
        default:
            throw std::runtime_error("Unexpected message while loading history");
        }
    }

private:
    ConsoleUi& ui_;
    bool history_finished_ = false;
};


class ReceivingMessageHandler final {
public:
    ReceivingMessageHandler(const WillClient& client, ConsoleUi& ui)
        : client_(client)
        , ui_(ui)
    {}

    void on(const v1::ServerEvent& event)
    {
        switch (event.event_case()) {
        case v1::ServerEvent::kAuthOk:
        case v1::ServerEvent::kAuthRequired:
        case v1::ServerEvent::kHistoryEnd:
            return;
        case v1::ServerEvent::kReceiptAck:
            if (!client_.config().quiet_receipts)
                ui_.print_receipt();
            return;
        case v1::ServerEvent::kHistoryItem:
            print_history_item(ui_, event.history_item());
            return;
        case v1::ServerEvent::kChat:
            ui_.print_peer(event.chat().name(), event.chat().body());
            return;
        case v1::ServerEvent::EVENT_NOT_SET:
            break;
        }

        throw std::runtime_error("Will protocol: unhandled server message type");
    }

private:
    const WillClient& client_;
    ConsoleUi& ui_;
};


template<typename Handler>
void on_server_event(const v1::ServerEvent& event, Handler& handler)
{
    if (!is_post_auth_server_event(event))
        throw std::runtime_error("Will protocol: invalid server frame");

    handler.on(event);
}


} // namespace


ChatSession::ChatSession(WillClient& client, ConsoleUi& ui)
    : client_(client)
    , ui_(ui)
{}


void ChatSession::run()
{
    loadHistory();

    std::atomic<bool> disconnected{false};

    client_.set_closed_handler([&disconnected] { disconnected.store(true); });
    client_.set_inbound_handler([this, &disconnected](const v1::ServerEvent& event) {
        if (disconnected.load())
            return;

        try {
            ReceivingMessageHandler handler{client_, ui_};
            on_server_event(event, handler);
        }
        catch (const std::exception& e) {
            ui_.set_live_prompt(false);
            ui_.print_error(std::string("Receive error: ") + e.what());
            disconnected.store(true);
        }
    });

    ui_.print_status("Connected to Will chat. Type messages and press Enter.");
    ui_.print_status("Press Ctrl+D to exit.");
    ui_.set_live_prompt(true);
    ui_.print_prompt();

    std::string line;
    while (!disconnected.load() && std::getline(std::cin, line)) {
        ui_.print_mine(line, false, !client_.config().quiet_receipts);
        client_.send(line);
    }

    ui_.set_live_prompt(false);

    if (disconnected.load())
        ui_.print_status("Disconnected from chat.");

    client_.shutdown();
}


void ChatSession::loadHistory() const
{
    if (client_.config().history_limit == 0)
        return;

    std::mutex mutex;
    std::condition_variable cv;
    bool finished = false;
    bool disconnected = false;
    std::string error_message;

    ui_.print_history_begin();

    client_.set_closed_handler([&] {
        std::lock_guard lock(mutex);
        if (!finished) {
            disconnected = true;
            cv.notify_one();
        }
    });

    client_.set_inbound_handler([&](const v1::ServerEvent& event) {
        try {
            LoadingHistoryMessageHandler handler{ui_};
            on_server_event(event, handler);

            std::lock_guard lock(mutex);
            if (handler.history_finished())
                finished = true;
            cv.notify_one();
        }
        catch (const std::exception& e) {
            std::lock_guard lock(mutex);
            error_message = e.what();
            disconnected = true;
            cv.notify_one();
        }
    });

    client_.requestHistory(client_.config().history_limit);

    std::unique_lock lock(mutex);
    cv.wait(lock, [&] { return finished || disconnected; });

    client_.set_closed_handler(nullptr);
    client_.set_inbound_handler(nullptr);

    if (!finished)
        throw std::runtime_error(disconnected && !error_message.empty() ? error_message
                                                                        : "Disconnected while loading history");

    ui_.print_history_end();
}


} // namespace will


namespace {


will::ClientConfig parse_client_config(int argc, char* argv[])
{
    will::ClientConfig config;
    CLI::App app{"will-client"};
    app.allow_extras(false);

    app.add_option("--host", config.host)->description("Server IPv4 address");
    app.add_option("--port", config.port)->description("Server TCP port");
    app.add_option("--device-token-path", config.device_token_path)
        ->description("Path to persistent device token file");
    app.add_flag("--quiet", config.quiet_receipts)->description("Suppress delivery acknowledgements");
    app.add_option("--history", config.history_limit)->description("Request last N messages on connect");
    app.add_flag_callback("--no-history", [&]() { config.history_limit = 0; })
        ->description("Do not request chat history on connect");
    app.add_option("--color", config.color)
        ->description("Color output: auto, always, or never (default auto)")
        ->transform(CLI::CheckedTransformer(std::map<std::string, will::ColorMode>{
            {"auto", will::ColorMode::Auto},
            {"always", will::ColorMode::Always},
            {"never", will::ColorMode::Never},
        }));

    try {
        app.parse(argc, argv);
    } catch (const CLI::CallForHelp& error) {
        std::cerr << std::format(
            "Usage: will-client [options]\n"
            "\n"
            "Options:\n"
            "  -h, --help                      Print usage and exit\n"
            "  --host HOST                     Server IPv4 address (default {}; Novosibirsk: {})\n"
            "  --port PORT                     Server TCP port (default {})\n"
            "  --device-token-path PATH        Device token file (default {})\n"
            "  --quiet                         Suppress delivery checkmarks on [me] lines\n"
            "  --history N                     Request last N messages on connect (default {})\n"
            "  --no-history                    Do not request chat history on connect\n"
            "  --color WHEN                    Color output: auto, always, never (default auto)\n",
            will::ClientConfig::DefaultHost,
            will::ClientConfig::NovosibirskHost,
            will::ClientConfig::DefaultPort,
            will::ClientConfig::DefaultDeviceTokenPath,
            will::ClientConfig::DefaultHistoryLimit);
        std::exit(error.get_exit_code());
    } catch (const CLI::ParseError& error) {
        std::exit(app.exit(error, std::cerr));
    }

    return config;
}


} // namespace


int main(int argc, char* argv[])
{
    will::ColorMode color = will::ColorMode::Auto;
    try {
        const will::ClientConfig config = parse_client_config(argc, argv);
        color = config.color;
        will::ConsoleUi ui(color);

        const std::string device_token = will::DeviceTokenStore::load_or_create(config.device_token_path);

        will::WillClient client(config);
        client.connect();
        client.authenticate_device(device_token);

        will::ChatSession chat_session(client, ui);
        chat_session.run();

        return 0;
    }
    catch (const std::exception& e) {
        will::ConsoleUi{color}.print_error(std::string("Client error: ") + e.what());
        return 1;
    }
}
