#include "chatsession.h"
#include "clientconfigparser.h"
#include "consoleui.h"
#include "willclient.h"

#include <string>


using namespace will;


int main(int argc, char* argv[])
{
    ColorMode color = will::ColorMode::Auto;

    try {
        const ClientConfigParser cli(argc, argv);
        color = cli.client_config().color;
        ConsoleUi ui(color);

        WillClient client(cli.client_config());
        client.connect();

        ChatSession chat_session(client, ui);
        chat_session.run();

        return 0;
    }
    catch (const std::exception& e) {
        ConsoleUi{color}.print_error(std::string("Client error: ") + e.what());
        return 1;
    }
}
