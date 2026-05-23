#include "serverconfigparser.h"

#include "cliparsercontext.h"
#include "serverclioptions.h"

#include <array>
#include <iostream>


namespace will {


namespace {


const std::array<const ServerCliOption*, 6>& all_server_cli_options()
{
    static const std::array<const ServerCliOption*, 6> options{
        &PortCliOption::instance(),
        &IoThreadsCliOption::instance(),
        &ListenBacklogCliOption::instance(),
        &MaxClientsCliOption::instance(),
        &MaxOutboundQueueCliOption::instance(),
        &HelpCliOption::instance(),
    };
    return options;
}


const std::array<const ServerCliOption*, 5>& config_server_cli_options()
{
    static const std::array<const ServerCliOption*, 5> options{
        &PortCliOption::instance(),
        &IoThreadsCliOption::instance(),
        &ListenBacklogCliOption::instance(),
        &MaxClientsCliOption::instance(),
        &MaxOutboundQueueCliOption::instance(),
    };
    return options;
}


} // namespace


ServerConfigParser::ServerConfigParser(int argc, char* argv[])
{
    parse_command_line(argc, argv);
}


void ServerConfigParser::print_usage()
{
    std::cerr << "Usage: will-server [options]\n";
    print_option_usage(std::cerr);
}


void ServerConfigParser::print_option_usage(std::ostream& os)
{
    print_cli_usage(os, config_server_cli_options());
}


const ServerCliOption* ServerConfigParser::find_option(std::string_view text)
{
    return find_cli_option(all_server_cli_options(), text);
}


void ServerConfigParser::parse_command_line(int argc, char* argv[])
{
    CliParserContext context(argc, argv);

    for (context.set_index(1); context.index() < context.argc();
         context.set_index(context.index() + 1)) {
        const std::string_view option_text = context.current();
        const ServerCliOption* const option = find_option(option_text);

        if (!option) {
            std::cerr << "Unknown option: " << option_text << '\n';
            print_usage();
            std::exit(2);
        }

        try {
            option->apply(context, config_);
        }
        catch (const std::exception& error) {
            context.cli_fail_option(option->primary_flag(), error);
        }
    }
}


} // namespace will
