#include "serverconfigparser.h"

#include "servercliapp.h"

#include <cstdlib>


namespace will {


ServerConfigParser::ServerConfigParser(int argc, char* argv[])
{
    cli::ServerCliApp cli;

    try {
        server_config_ = cli.parse(argc, argv);
    } catch (const CLI::CallForHelp& error) {
        cli.exit_on_help(error);
    } catch (const CLI::ParseError& error) {
        cli.exit_on_parse_error(error);
    }
}


const ServerConfig& ServerConfigParser::server_config() const noexcept
{
    return server_config_;
}


} // namespace will
