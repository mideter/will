#include "loadclientsconfigparser.h"

#include "loadclientscliapp.h"

#include <cstdlib>


namespace will {


LoadClientsConfigParser::LoadClientsConfigParser(int argc, char* argv[])
{
    LoadClientsCliApp cli;

    try {
        load_config_ = cli.parse(argc, argv);
    } catch (const CLI::CallForHelp& error) {
        cli.exit_on_help(error);
    } catch (const CLI::ParseError& error) {
        cli.exit_on_parse_error(error);
    }
}


const LoadClientsConfig& LoadClientsConfigParser::load_config() const noexcept
{
    return load_config_;
}


} // namespace will
