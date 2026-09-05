#include "serverapp.h"

#include "serverconfigvalidator.h"
#include "willserver.h"

#include <cstdlib>
#include <iostream>


namespace will {


ServerApp::ServerApp(int argc, char* argv[])
	: cli_{}
{
	try {
		config_ = ServerConfigValidator::accept(cli_.parse(argc, argv));
	} catch (const CLI::CallForHelp& error) {
		cli_.exit_on_help(error);
	} catch (const CLI::ParseError& error) {
		cli_.exit_on_parse_error(error);
	} catch (const ServerConfigError& error) {
		std::cerr << "Server error: " << error.what() << '\n';
		std::exit(EXIT_FAILURE);
	}
}


const ServerConfig& ServerApp::config() const noexcept
{
	return config_;
}


int ServerApp::run()
try {
	WillServer server(config_);
	server.run();

	return 0;
}
catch (const std::exception& e) {
	std::cerr << "Server error: " << e.what() << '\n';
	return 1;
}


} // namespace will
