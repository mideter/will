#include <CLI/CLI.hpp>

#include <cassert>
#include <cstdlib>
#include <sstream>
#include <string>
#include <string_view>
#include <vector>

import will.client.clientcliapp;
import will.client.clientconfig;


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
    ClientCliApp cli;
    bool threw = false;
    try {
        (void)cli.parse(argv.argc(), argv.argv());
    } catch (const CLI::ParseError&) {
        threw = true;
    }
    assert(threw);
}


void assert_help_requested(std::initializer_list<const char*> args)
{
    using namespace will;

    Argv argv{args};
    ClientCliApp cli;
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
    ClientCliApp{}.print_help(out);
    const std::string help = out.str();

    assert(help.find("Usage: will-client") != std::string::npos);
    assert(help.find("--host") != std::string::npos);
    assert(help.find(will::ClientConfig::NovosibirskHost) != std::string::npos);
    assert(help.find("--port") != std::string::npos);
    assert(help.find("--quiet") != std::string::npos);
    assert(help.find("--device-token-path") != std::string::npos);
    assert(help.find("--no-history") != std::string::npos);
    assert(help.find("--color") != std::string::npos);
    assert(help.find("-h, --help") != std::string::npos);
}


void assert_defaults()
{
    using namespace will;

    Argv args{"will-client"};
    const will::ClientConfig config = ClientCliApp{}.parse(args.argc(), args.argv());

    assert(config.host == will::ClientConfig::DefaultHost);
    assert(config.port == will::ClientConfig::DefaultPort);
    assert(config.device_token_path == will::ClientConfig::DefaultDeviceTokenPath);
    assert(config.quiet_receipts == will::ClientConfig::DefaultQuietReceipts);
    assert(config.history_limit == will::ClientConfig::DefaultHistoryLimit);
    assert(config.color == will::ClientConfig::DefaultColor);
}


void assert_no_history()
{
    using namespace will;

    Argv args{"will-client", "--no-history"};
    const will::ClientConfig config = ClientCliApp{}.parse(args.argc(), args.argv());

    assert(config.history_limit == 0);
}


void assert_all_options()
{
    using namespace will;

    Argv args{"will-client", "--host", "192.168.1.10", "--port", "9000", "--device-token-path",
              "/tmp/token", "--quiet", "--history", "25", "--color", "never"};
    const will::ClientConfig config = ClientCliApp{}.parse(args.argc(), args.argv());

    assert(config.host == "192.168.1.10");
    assert(config.port == 9000);
    assert(config.device_token_path == "/tmp/token");
    assert(config.quiet_receipts);
    assert(config.history_limit == 25u);
    assert(config.color == ColorMode::Never);
}


void assert_color_always()
{
    using namespace will;

    Argv args{"will-client", "--color", "always"};
    const will::ClientConfig config = ClientCliApp{}.parse(args.argc(), args.argv());
    assert(config.color == ColorMode::Always);
}


void assert_port_zero_parses()
{
    using namespace will;

    Argv args{"will-client", "--port", "0"};
    const will::ClientConfig config = ClientCliApp{}.parse(args.argc(), args.argv());
    assert(config.port == 0);
}


} // namespace


int main()
{
    assert_help_output();
    assert_parse_error({"will-client", "--unknown"});
    assert_help_requested({"will-client", "--help"});
    assert_help_requested({"will-client", "-h"});
    assert_help_requested({"will-client", "--help", "--port", "8080"});
    assert_help_requested({"will-client", "--port", "8080", "--help"});
    assert_defaults();
    assert_no_history();
    assert_all_options();
    assert_color_always();
    assert_port_zero_parses();
    assert_parse_error({"will-client", "--port"});
    assert_parse_error({"will-client", "--color", "rainbow"});

    return EXIT_SUCCESS;
}
