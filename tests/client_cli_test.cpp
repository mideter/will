#include "clientcliapp.h"
#include "clientconfig.h"

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
    assert(help.find("-h, --help") != std::string::npos);
}


void assert_defaults()
{
    using namespace will;

    Argv args{"will-client"};
    const will::ClientConfig config = ClientCliApp{}.parse(args.argc(), args.argv());

    assert(config.host == will::ClientConfig::DefaultHost);
    assert(config.port == will::ClientConfig::DefaultPort);
    assert(config.quiet_receipts == will::ClientConfig::DefaultQuietReceipts);
}


void assert_all_options()
{
    using namespace will;

    Argv args{"will-client", "--host", "192.168.1.10", "--port", "9000", "--quiet"};
    const will::ClientConfig config = ClientCliApp{}.parse(args.argc(), args.argv());

    assert(config.host == "192.168.1.10");
    assert(config.port == 9000);
    assert(config.quiet_receipts);
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
    assert_all_options();
    assert_port_zero_parses();
    assert_parse_error({"will-client", "--port"});

    return EXIT_SUCCESS;
}
