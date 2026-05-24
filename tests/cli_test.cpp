#include "clioption.h"
#include "clioptioncursor.h"
#include "serverclioption.h"
#include "serverconfig.h"

#include <cassert>
#include <cstdlib>
#include <string>
#include <vector>


namespace {


struct Argv {
    std::vector<std::string> storage;
    std::vector<char*> ptrs;

    explicit Argv(std::initializer_list<const char*> args)
    {
        storage.reserve(args.size());
        for (const char* arg : args)
            storage.emplace_back(arg);

        ptrs.reserve(storage.size());
        for (std::string& arg : storage)
            ptrs.push_back(arg.data());

        ptrs.push_back(nullptr);
    }

    int argc() const { return static_cast<int>(storage.size()); }

    char** argv() { return ptrs.data(); }
};


struct HelpRequested {};


will::ServerConfig parse_server_options(int argc, char* argv[]);


void assert_help_matches()
{
    using namespace will::cli;

    assert(ServerOptionTable::HelpOption.matches("--help"));
    assert(ServerOptionTable::HelpOption.matches("-h"));
    assert(!ServerOptionTable::HelpOption.matches("--port"));
}


void assert_unknown_option()
{
    using namespace will::cli;

    Argv args{"will-server", "--unknown"};
    OptionCursor cursor(args.argc(), args.argv(), ServerOptionTable::ServerOptions);

    bool threw = false;
    try {
        (void)cursor.match();
    } catch (const UnknownOptionError& error) {
        threw = true;
        assert(std::string_view{error.what()}.find("--unknown") != std::string_view::npos);
    }
    assert(threw);
}


void assert_help_not_alone()
{
    using namespace will::cli;

    Argv args{"will-server", "--help", "--port", "8080"};
    bool threw = false;
    try {
        (void)parse_server_options(args.argc(), args.argv());
    } catch (const HelpNotAloneError&) {
        threw = true;
    }
    assert(threw);
}


void assert_help_only_when_alone()
{
    using namespace will::cli;

    Argv args{"will-server", "--help"};
    bool threw = false;
    try {
        (void)parse_server_options(args.argc(), args.argv());
    } catch (const HelpRequested&) {
        threw = true;
    }
    assert(threw);
}


void assert_help_after_other_option_is_unknown()
{
    using namespace will::cli;

    Argv args{"will-server", "--port", "8080", "--help"};
    bool threw = false;
    try {
        (void)parse_server_options(args.argc(), args.argv());
    } catch (const UnknownOptionError& error) {
        threw = true;
        assert(std::string_view{error.what()}.find("--help") != std::string_view::npos);
    }
    assert(threw);
}


will::ServerConfig parse_server_options(int argc, char* argv[])
{
    using namespace will::cli;

    will::ServerConfig config;
    OptionCursor cursor(argc, argv, ServerOptionTable::ServerOptions);

    if (cursor.has_option()
        && ServerOptionTable::HelpOption.matches(cursor.current_option())) {
        if (argc != 2)
            throw HelpNotAloneError{};

        throw HelpRequested{};
    }

    while (cursor.has_option()) {
        const OptionMatch<ServerOption> match = cursor.match();
        try {
            std::visit([&](const auto& option) { option.apply(config, match.value()); }, match.option());
        } catch (const will::ServerConfigError& error) {
            throw InvalidOptionError(match.primary_flag(), error.what());
        }
        cursor.advance();
    }

    return config;
}


void assert_defaults()
{
    using namespace will::cli;

    Argv args{"will-server"};
    const will::ServerConfig config = parse_server_options(args.argc(), args.argv());

    assert(config.listen_port() == will::ServerConfig::DefaultListenPort);
    assert(config.io_threads() == will::ServerConfig::DefaultIoThreads);
    assert(config.listen_backlog() == will::ServerConfig::DefaultListenBacklog);
    assert(config.max_connections() == will::ServerConfig::DefaultMaxConnections);
    assert(config.max_outbound_queue_bytes() == will::ServerConfig::DefaultMaxOutboundQueueBytes);
}


void assert_all_options()
{
    using namespace will::cli;

    Argv args{"will-server",
              "--port",
              "9000",
              "--io-threads",
              "2",
              "--listen-backlog",
              "512",
              "--max-clients",
              "128",
              "--max-outbound-queue-bytes",
              "4096"};
    const will::ServerConfig config = parse_server_options(args.argc(), args.argv());

    assert(config.listen_port() == 9000);
    assert(config.io_threads() == 2);
    assert(config.listen_backlog() == 512);
    assert(config.max_connections() == 128);
    assert(config.max_outbound_queue_bytes() == 4096);
}


void assert_invalid_port()
{
    using namespace will::cli;

    Argv args{"will-server", "--port", "0"};
    bool threw = false;
    try {
        (void)parse_server_options(args.argc(), args.argv());
    } catch (const InvalidOptionError& error) {
        threw = true;
        const std::string_view message{error.what()};
        assert(message.find("--port") != std::string_view::npos);
        assert(message.find("listen_port") != std::string_view::npos);
    }
    assert(threw);
}


void assert_missing_value()
{
    using namespace will::cli;

    Argv args{"will-server", "--port"};
    bool threw = false;
    try {
        (void)parse_server_options(args.argc(), args.argv());
    } catch (const InvalidOptionError& error) {
        threw = true;
        const std::string_view message{error.what()};
        assert(message.find("--port") != std::string_view::npos);
        assert(message.find("requires a value") != std::string_view::npos);
    }
    assert(threw);
}


void assert_invalid_numeric_value()
{
    using namespace will::cli;

    Argv args{"will-server", "--io-threads", "abc"};
    bool threw = false;
    try {
        (void)parse_server_options(args.argc(), args.argv());
    } catch (const InvalidOptionError& error) {
        threw = true;
        const std::string_view message{error.what()};
        assert(message.find("--io-threads") != std::string_view::npos);
        assert(message.find("invalid value") != std::string_view::npos);
    }
    assert(threw);
}


void assert_cli_option_read_value()
{
    using namespace will::cli;

    Argv args{"will-server", "--port", "1234"};
    OptionCursor cursor(args.argc(), args.argv(), ServerOptionTable::ServerOptions);

    const OptionMatch<ServerOption> match = cursor.match();
    assert(match.primary_flag() == "--port");
    assert(std::holds_alternative<int>(match.value()));
    assert(std::get<int>(match.value()) == 1234);
}


} // namespace


int main()
{
    assert_help_matches();
    assert_unknown_option();
    assert_help_not_alone();
    assert_help_only_when_alone();
    assert_help_after_other_option_is_unknown();
    assert_defaults();
    assert_all_options();
    assert_invalid_port();
    assert_missing_value();
    assert_invalid_numeric_value();
    assert_cli_option_read_value();

    return EXIT_SUCCESS;
}
