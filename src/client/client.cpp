#include "entities/auth_token.h"
#include "chatsession.h"
#include "clientconfigparser.h"
#include "consoleui.h"
#include "devicetokenstore.h"
#include "willclient.h"

#include <string>


int main(int argc, char* argv[])
{
    will::ColorMode color = will::ColorMode::Auto;
    try {
        const will::ClientConfigParser cli(argc, argv);
        color = cli.client_config().color;
        will::ConsoleUi ui(color);

        const will::domain::AuthToken device_token =
            will::DeviceTokenStore::load_or_create(cli.client_config().device_token_path);

        will::WillClient client(cli.client_config());
        client.connect();
        client.authenticate_device(device_token.value);

        will::ChatSession chat_session(client, ui);
        chat_session.run();

        return 0;
    }
    catch (const std::exception& e) {
        will::ConsoleUi{color}.print_error(std::string("Client error: ") + e.what());
        return 1;
    }
}
