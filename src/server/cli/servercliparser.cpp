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
    
    for (const CliOption& option : ServerCliOptions())
        option.print_usage(std::cerr);

    HelpCliOption.print_usage(std::cerr);
}


void ServerCliParser::apply_matched_option(const CliOptionMatch& option)
try {
    server_config_.apply_cli_option(option);
}
catch (const std::exception& error) {
    cli_fail_option(option.primary_flag(), error);
}


void ServerCliParser::fail_unknown_option(const CliOptionMatch& option)
{
    std::cerr << "Unknown option: " << option.token() << '\n';
    print_usage();
    std::exit(2);
}


void ServerCliParser::fail_help_not_alone()
{
    std::cerr << "--help must be the only option\n";
    print_usage();
    std::exit(2);
}


void ServerCliParser::exit_with_help()
{
    print_usage();
    std::exit(0);
}


void ServerCliParser::cli_fail_option(std::string_view flag, const std::exception& error)
{
    std::cerr << "Invalid " << flag << ": " << error.what() << '\n';
    std::exit(2);
}


void ServerCliParser::handle_help_option(int argc, CliCursor& cursor)
{
    if (!cursor.has_option() || !HelpCliOption.matches(cursor.current_option()))
        return;

    if (argc != 2)
        fail_help_not_alone();

    exit_with_help();
}


void ServerCliParser::parse_command_line(int argc, char* argv[])
{
    CliCursor cursor(argc, argv);

    cursor.begin_options();
    handle_help_option(argc, cursor);

    while (cursor.has_option()) {
        const CliOptionMatch option = cursor.get_option(ServerCliOptions());

        if (!option)
            fail_unknown_option(option);

        apply_matched_option(option);
        cursor.next_option();
    }
}


} // namespace will
