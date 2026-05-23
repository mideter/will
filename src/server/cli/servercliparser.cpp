#include "servercliparser.h"

#include "clicursor.h"
#include "serverclioptions.h"

#include <cstdlib>
#include <exception>
#include <iostream>


namespace will {


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


void ServerCliParser::cli_fail_option(std::string_view flag, const std::exception& error)
{
    std::cerr << "Invalid " << flag << ": " << error.what() << '\n';
    std::exit(2);
}


void ServerCliParser::parse_command_line(int argc, char* argv[])
{
    CliCursor cursor(argc, argv);

    cursor.begin_options();
    while (cursor.has_option()) {
        const CliOptionMatch option = cursor.get_option(all_server_cli_options());

        if (!option) {
            std::cerr << "Unknown option: " << option.token() << '\n';
            print_usage();
            std::exit(2);
        }

        const std::string_view token = option.token();
        if (token == "--help" || token == "-h") {
            print_usage();
            std::exit(0);
        }

        try {
            server_config_.apply_cli_option(option);
        }
        catch (const std::exception& error) {
            cli_fail_option(option.primary_flag(), error);
        }

        cursor.next_option();
    }
}


} // namespace will
