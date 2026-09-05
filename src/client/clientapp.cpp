#include "clientapp.h"

#include "chatsession.h"
#include "clientconfigvalidator.h"
#include "consoleui.h"
#include "willclient.h"

#include <cstdlib>
#include <string>


namespace will {


ClientApp::ClientApp(int argc, char* argv[])
	: cli_{}
{
	try {
		config_ = ClientConfigValidator::accept(cli_.parse(argc, argv));
	} catch (const CLI::CallForHelp& error) {
		cli_.exit_on_help(error);
	} catch (const CLI::ParseError& error) {
		cli_.exit_on_parse_error(error);
	} catch (const ClientConfigError& error) {
		ConsoleUi{}.print_error(std::string("Client error: ") + error.what());
		std::exit(EXIT_FAILURE);
	}
}


const ClientConfig& ClientApp::config() const noexcept
{
	return config_;
}


int ClientApp::run()
{
	try {
		ConsoleUi ui(config_.color);

		WillClient client(config_);
		client.connect();

		ChatSession chat_session(client, ui);
		chat_session.run();

		return 0;
	} catch (const std::exception& e) {
		ConsoleUi{config_.color}.print_error(std::string("Client error: ") + e.what());
		return 1;
	}
}


} // namespace will
