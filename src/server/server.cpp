#include <CLI/CLI.hpp>

#include <exception>
#include <iostream>
#include <utility>

import will.server.servercliapp;
import will.server.willserver;


int main(int argc, char* argv[])
try {
    will::ServerCliApp cli;
    will::ServerConfig config;
    try {
        config = cli.parse(argc, argv);
    } catch (const CLI::CallForHelp& error) {
        cli.exit_on_help(error);
    } catch (const CLI::ParseError& error) {
        cli.exit_on_parse_error(error);
    }

    will::WillServer server(std::move(config));
    server.run();
    return 0;
}
catch (const std::exception& e) {
    std::cerr << "Server error: " << e.what() << '\n';
    return 1;
}
