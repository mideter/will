#include "servercliparser.h"

#include "clicursor.h"
#include "serverclioptions.h"

#include <cstdlib>
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


void ServerCliParser::exit_with_help()
{
    print_usage();
    std::exit(0);
}


void ServerCliParser::handle_help_option(int argc, CliCursor& cursor)
{
    if (!cursor.has_option() || !HelpCliOption.matches(cursor.current_option()))
        return;

    if (argc != 2)
        throw CliHelpNotAloneError{};

    exit_with_help();
}


void ServerCliParser::parse_command_line(int argc, char* argv[])
try {
    CliCursor cursor(argc, argv);

    cursor.begin_options();
    handle_help_option(argc, cursor);

    while (cursor.has_option()) {
        server_config_.apply_cli_option(CliOptionMatch::parse(cursor, ServerCliOptions()));
        cursor.next_option();
    }
}
catch (const CliInvalidOptionError& error) {
    std::cerr << error.what() << '\n';
    std::exit(2);
}
catch (const CliError& error) {
    std::cerr << error.what() << '\n';
    print_usage();
    std::exit(2);
}


} // namespace will
