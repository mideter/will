#include "chatsession.h"
#include "clientconfigparser.h"
#include "consoleui.h"
#include "devicetokenstore.h"
#include "entities/device_token.h"
#include "willclient.h"

#include <string>


using namespace will;
using domain::DeviceToken;


int main(int argc, char* argv[])
{
    ColorMode color = will::ColorMode::Auto;

    try {
        const ClientConfigParser cli(argc, argv);
        color = cli.client_config().color;
        ConsoleUi ui(color);

        const DeviceToken device_token =
            DeviceTokenStore::load_or_create(cli.client_config().device_token_path);

        WillClient client(cli.client_config());
        client.connect();
        client.authenticate_device(device_token.text());

        ChatSession chat_session(client, ui);
        chat_session.run();

        return 0;
    }
    catch (const std::exception& e) {
        ConsoleUi{color}.print_error(std::string("Client error: ") + e.what());
        return 1;
    }
}
