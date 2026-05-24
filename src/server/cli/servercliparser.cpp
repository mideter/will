#include "servercliparser.h"

#include <cstdlib>
#include <iostream>


namespace will {
namespace cli {


ServerParser::ServerParser(int argc, char* argv[])
{
    parse_command_line(argc, argv);
}


const ServerConfig& ServerParser::server_config() const noexcept
{
    return server_config_;
}


void ServerParser::print_usage()
{
    std::cerr << "Usage: will-server [options]\n";

    for (const ServerOption& option : ServerOptionTable::ServerOptions) {
        std::visit([](const auto& entry) { entry.print_usage(std::cerr); }, option);
    }

    ServerOptionTable::HelpOption.print_usage(std::cerr);
}


void ServerParser::handle_help_option(int argc, OptionCursorCore& cursor)
{
    if (!cursor.has_option()
        || !ServerOptionTable::HelpOption.matches(cursor.current_option())) {
        return;
    }

    if (argc != 2)
        throw HelpNotAloneError{};

    print_usage();
    std::exit(0);
}


void ServerParser::apply_option(const OptionMatch<ServerOption>& match)
try {
    std::visit([&](const auto& option) { option.apply(server_config_, match.value()); }, match.option());
} catch (const ServerConfigError& error) {
    throw InvalidOptionError(match.primary_flag(), error.what());
}


void ServerParser::parse_command_line(int argc, char* argv[])
try {
    OptionCursor cursor(argc, argv, ServerOptionTable::ServerOptions);

    handle_help_option(argc, cursor);

    while (cursor.has_option()) {
        apply_option(cursor.match());
        cursor.advance();
    }
}
catch (const Error& error) {
    std::cerr << error.what() << '\n';
    print_usage();
    std::exit(2);
}


} // namespace cli
} // namespace will
