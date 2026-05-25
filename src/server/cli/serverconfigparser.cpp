#include "serverconfigparser.h"

#include "servercliapp.h"

#include <cstdlib>
#include <iostream>


namespace will {
namespace cli {


ServerConfigParser::ServerConfigParser(int argc, char* argv[])
{
    ServerCliApp cli;

    try {
        server_config_ = cli.parse(argc, argv);
    } catch (const CLI::CallForHelp& error) {
        cli.exit_on_help(error);
    } catch (const CLI::ParseError& error) {
        cli.exit_on_parse_error(error);
    } catch (const std::runtime_error& error) {
        std::cerr << error.what() << '\n';
        cli.print_help(std::cerr);
        std::exit(2);
    }
}


const ServerConfig& ServerConfigParser::server_config() const noexcept
{
    return server_config_;
}


} // namespace cli
} // namespace will
