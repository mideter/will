#include "servercliparser.h"

#include <cstdlib>
#include <iostream>


namespace will {


ServerCliParser::ServerCliParser(int argc, char* argv[])
{
    parse_command_line(argc, argv);
}


const ServerConfig& ServerCliParser::server_config() const noexcept
{
    return server_config_;
}


void ServerCliParser::print_usage()
{
    std::cerr << "Usage: will-server [options]\n";

    for (const ServerOption& option : ServerCliOptionTable::ServerOptions) {
        std::visit([](const auto& entry) { entry.print_usage(std::cerr); }, option);
    }

    ServerCliOptionTable::HelpOption.print_usage(std::cerr);
}


void ServerCliParser::handle_help_option(int argc, ServerCliOptionCursor& cursor)
{
    if (!cursor.has_option()
        || !ServerCliOptionTable::HelpOption.matches(cursor.current_option())) {
        return;
    }

    if (argc != 2)
        throw CliHelpNotAloneError{};

    print_usage();
    std::exit(0);
}


void ServerCliParser::apply_cli_option(const CliOptionMatch<ServerOption>& match)
try {
    std::visit([&](const auto& option) { option.apply(server_config_, match.value()); }, match.option());
} catch (const ServerConfigError& error) {
    throw CliInvalidOptionError(match.primary_flag(), error.what());
}


void ServerCliParser::parse_command_line(int argc, char* argv[])
try {
    ServerCliOptionCursor cursor(argc, argv);

    handle_help_option(argc, cursor);

    while (cursor.has_option()) {
        apply_cli_option(cursor.match());
        cursor.advance();
    }
}
catch (const CliError& error) {
    std::cerr << error.what() << '\n';
    print_usage();
    std::exit(2);
}


} // namespace will
