#include "servercliapp.h"
#include "serverconfig.h"

#include <CLI/CLI.hpp>

#include <cassert>
#include <cstdlib>
#include <string>
#include <string_view>
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


void assert_parse_error(std::initializer_list<const char*> args)
{
    using namespace will::cli;

    Argv argv{args};
    ServerCliApp cli;
    bool threw = false;
    try {
        (void)cli.parse(argv.argc(), argv.argv());
    } catch (const CLI::ParseError&) {
        threw = true;
    }
    assert(threw);
}


void assert_runtime_error(std::initializer_list<const char*> args, std::string_view fragment)
{
    using namespace will::cli;

    Argv argv{args};
    ServerCliApp cli;
    bool threw = false;
    try {
        (void)cli.parse(argv.argc(), argv.argv());
    } catch (const std::runtime_error& error) {
        threw = true;
        assert(std::string_view{error.what()}.find(fragment) != std::string_view::npos);
    }
    assert(threw);
}


void assert_help_requested(std::initializer_list<const char*> args)
{
    using namespace will::cli;

    Argv argv{args};
    ServerCliApp cli;
    bool threw = false;
    try {
        (void)cli.parse(argv.argc(), argv.argv());
    } catch (const CLI::CallForHelp&) {
        threw = true;
    }
    assert(threw);
}


void assert_defaults()
{
    using namespace will::cli;

    Argv args{"will-server"};
    const will::ServerConfig config = ServerCliApp{}.parse(args.argc(), args.argv());

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
    const will::ServerConfig config = ServerCliApp{}.parse(args.argc(), args.argv());

    assert(config.listen_port() == 9000);
    assert(config.io_threads() == 2);
    assert(config.listen_backlog() == 512);
    assert(config.max_connections() == 128);
    assert(config.max_outbound_queue_bytes() == 4096);
}


} // namespace


int main()
{
    assert_parse_error({"will-server", "--unknown"});
    assert_help_requested({"will-server", "--help"});
    assert_help_requested({"will-server", "-h"});
    assert_help_requested({"will-server", "--help", "--port", "8080"});
    assert_help_requested({"will-server", "--port", "8080", "--help"});
    assert_defaults();
    assert_all_options();
    assert_runtime_error({"will-server", "--port", "0"}, "Invalid --port:");
    assert_runtime_error({"will-server", "--port", "0"}, "listen_port");
    assert_parse_error({"will-server", "--port"});
    assert_parse_error({"will-server", "--io-threads", "abc"});

    return EXIT_SUCCESS;
}
