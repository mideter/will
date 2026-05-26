#include "clientconfigparser.h"

#include "clientcliapp.h"

#include <cstdlib>


namespace will {


ClientConfigParser::ClientConfigParser(int argc, char* argv[])
{
    ClientCliApp cli;

    try {
        client_config_ = cli.parse(argc, argv);
    } catch (const CLI::CallForHelp& error) {
        cli.exit_on_help(error);
    } catch (const CLI::ParseError& error) {
        cli.exit_on_parse_error(error);
    }
}


const ClientConfig& ClientConfigParser::client_config() const noexcept
{
    return client_config_;
}


} // namespace will
