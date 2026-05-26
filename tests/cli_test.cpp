#include "servercliapp.h"
#include "serverconfig.h"

#include <CLI/CLI.hpp>

#include <cassert>
#include <cstdlib>
#include <sstream>
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
    using namespace will;

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
    using namespace will;

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
    using namespace will;

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


void assert_help_output()
{
    using namespace will;

    std::ostringstream out;
    ServerCliApp{}.print_help(out);
    const std::string help = out.str();

    assert(help.find("Usage: will-server") != std::string::npos);
    assert(help.find("--port") != std::string::npos);
    assert(help.find("-h, --help") != std::string::npos);
    assert(help.find("OPTIONS:") == std::string::npos);
    assert(help.find(" INT") == std::string::npos);
    assert(help.find(" UINT") == std::string::npos);
    assert(help.find("[OPTIONS]") == std::string::npos);
}


void assert_defaults()
{
    using namespace will;

    Argv args{"will-server"};
    const will::ServerConfig config = ServerCliApp{}.parse(args.argc(), args.argv());

    assert(config.listen_port == will::ServerConfig::DefaultListenPort);
    assert(config.io_threads == will::ServerConfig::DefaultIoThreads);
    assert(config.listen_backlog == will::ServerConfig::DefaultListenBacklog);
    assert(config.max_connections == will::ServerConfig::DefaultMaxConnections);
    assert(config.max_outbound_queue_bytes == will::ServerConfig::DefaultMaxOutboundQueueBytes);
    assert(config.db_path == will::ServerConfig::DefaultDbPath);
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
              "4096",
              "--db-path",
              "/tmp/custom.db"};
    const will::ServerConfig config = ServerCliApp{}.parse(args.argc(), args.argv());

    assert(config.listen_port == 9000);
    assert(config.io_threads == 2);
    assert(config.listen_backlog == 512);
    assert(config.max_connections == 128);
    assert(config.max_outbound_queue_bytes == 4096);
    assert(config.db_path == "/tmp/custom.db");
}


} // namespace


int main()
{
    assert_help_output();
    assert_parse_error({"will-server", "--unknown"});
    assert_help_requested({"will-server", "--help"});
    assert_help_requested({"will-server", "-h"});
    assert_help_requested({"will-server", "--help", "--port", "8080"});
    assert_help_requested({"will-server", "--port", "8080", "--help"});
    assert_defaults();
    assert_all_options();
    assert_parse_error({"will-server", "--port"});
    assert_parse_error({"will-server", "--io-threads", "abc"});

    return EXIT_SUCCESS;
}
