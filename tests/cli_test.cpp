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
    using namespace will;

    assert(ServerCliOptionTable::HelpOption.matches("--help"));
    assert(ServerCliOptionTable::HelpOption.matches("-h"));
    assert(!ServerCliOptionTable::HelpOption.matches("--port"));
}


void assert_unknown_option()
{
    using namespace will;

    Argv args{"will-server", "--unknown"};
    CliOptionCursor cursor(args.argc(), args.argv());

    bool threw = false;
    try {
        (void)CliOptionMatch<ServerOption>{cursor, ServerCliOptionTable::ServerOptions};
    } catch (const CliUnknownOptionError& error) {
        threw = true;
        assert(std::string_view{error.what()}.find("--unknown") != std::string_view::npos);
    }
    assert(threw);
}


void assert_help_not_alone()
{
    using namespace will;

    Argv args{"will-server", "--help", "--port", "8080"};
    bool threw = false;
    try {
        (void)parse_server_options(args.argc(), args.argv());
    } catch (const CliHelpNotAloneError&) {
        threw = true;
    }
    assert(threw);
}


void assert_help_only_when_alone()
{
    using namespace will;

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
    using namespace will;

    Argv args{"will-server", "--port", "8080", "--help"};
    bool threw = false;
    try {
        (void)parse_server_options(args.argc(), args.argv());
    } catch (const CliUnknownOptionError& error) {
        threw = true;
        assert(std::string_view{error.what()}.find("--help") != std::string_view::npos);
    }
    assert(threw);
}


will::ServerConfig parse_server_options(int argc, char* argv[])
{
    using namespace will;

    ServerConfig config;
    CliOptionCursor cursor(argc, argv);

    if (cursor.has_option()
        && ServerCliOptionTable::HelpOption.matches(cursor.current_option())) {
        if (argc != 2)
            throw CliHelpNotAloneError{};

        throw HelpRequested{};
    }

    while (cursor.has_option()) {
        const CliOptionMatch<ServerOption> match{cursor, ServerCliOptionTable::ServerOptions};
        try {
            std::visit([&](const auto& option) { option.apply(config, match.value()); }, match.option());
        } catch (const ServerConfigError& error) {
            throw CliInvalidOptionError(match.primary_flag(), error.what());
        }
        cursor++;
    }

    return config;
}


void assert_defaults()
{
    using namespace will;

    Argv args{"will-server"};
    const ServerConfig config = parse_server_options(args.argc(), args.argv());

    assert(config.listen_port() == ServerConfig::DefaultListenPort);
    assert(config.io_threads() == ServerConfig::DefaultIoThreads);
    assert(config.listen_backlog() == ServerConfig::DefaultListenBacklog);
    assert(config.max_connections() == ServerConfig::DefaultMaxConnections);
    assert(config.max_outbound_queue_bytes() == ServerConfig::DefaultMaxOutboundQueueBytes);
}


void assert_all_options()
{
    using namespace will;

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
    const ServerConfig config = parse_server_options(args.argc(), args.argv());

    assert(config.listen_port() == 9000);
    assert(config.io_threads() == 2);
    assert(config.listen_backlog() == 512);
    assert(config.max_connections() == 128);
    assert(config.max_outbound_queue_bytes() == 4096);
}


void assert_invalid_port()
{
    using namespace will;

    Argv args{"will-server", "--port", "0"};
    bool threw = false;
    try {
        (void)parse_server_options(args.argc(), args.argv());
    } catch (const CliInvalidOptionError& error) {
        threw = true;
        const std::string_view message{error.what()};
        assert(message.find("--port") != std::string_view::npos);
        assert(message.find("listen_port") != std::string_view::npos);
    }
    assert(threw);
}


void assert_missing_value()
{
    using namespace will;

    Argv args{"will-server", "--port"};
    bool threw = false;
    try {
        (void)parse_server_options(args.argc(), args.argv());
    } catch (const CliInvalidOptionError& error) {
        threw = true;
        const std::string_view message{error.what()};
        assert(message.find("--port") != std::string_view::npos);
        assert(message.find("requires a value") != std::string_view::npos);
    }
    assert(threw);
}


void assert_invalid_numeric_value()
{
    using namespace will;

    Argv args{"will-server", "--io-threads", "abc"};
    bool threw = false;
    try {
        (void)parse_server_options(args.argc(), args.argv());
    } catch (const CliInvalidOptionError& error) {
        threw = true;
        const std::string_view message{error.what()};
        assert(message.find("--io-threads") != std::string_view::npos);
        assert(message.find("invalid value") != std::string_view::npos);
    }
    assert(threw);
}


void assert_cli_option_read_value()
{
    using namespace will;

    Argv args{"will-server", "--port", "1234"};
    CliOptionCursor cursor(args.argc(), args.argv());

    const CliOptionMatch<ServerOption> match{cursor, ServerCliOptionTable::ServerOptions};
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
