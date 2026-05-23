#include "servercliparser.h"

#include "clicursor.h"
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


ServerCliParser::ServerCliParser(int argc, char* argv[])
{
    parse_command_line(argc, argv);
}


void ServerCliParser::print_usage()
{
    std::cerr << "Usage: will-server [options]\n";
    print_option_usage(std::cerr);
}


void ServerCliParser::print_option_usage(std::ostream& os)
{
    print_cli_usage(os, config_server_cli_options());
}


const ServerCliOption* ServerCliParser::find_option(std::string_view text)
{
    return find_cli_option(all_server_cli_options(), text);
}


void ServerCliParser::parse_command_line(int argc, char* argv[])
{
    CliCursor cursor(argc, argv);

    cursor.begin_options();
    while (cursor.has_option()) {
        const std::string_view option_text = cursor.current_option();
        const ServerCliOption* const option = find_option(option_text);

        if (!option) {
            std::cerr << "Unknown option: " << option_text << '\n';
            print_usage();
            std::exit(2);
        }

        try {
            option->apply(cursor, server_config_);
        }
        catch (const std::exception& error) {
            cursor.cli_fail_option(option->primary_flag(), error);
        }

        cursor.next_option();
    }
}


} // namespace will
