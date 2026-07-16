#include <iostream>

#include "entities/auth_token.h"
#include "chatsession.h"
#include "clientconfigparser.h"
#include "devicetokenstore.h"
#include "willclient.h"


int main(int argc, char* argv[])
try {
    const will::ClientConfigParser cli(argc, argv);
    const will::domain::AuthToken device_token =
        will::DeviceTokenStore::load_or_create(cli.client_config().device_token_path);

    will::WillClient client(cli.client_config());
    client.connect();
    client.authenticate_device(device_token.value);

    will::ChatSession chat_session(client);
    chat_session.run();

    return 0;
}
catch (const std::exception& e) {
    std::cerr << "Client error: " << e.what() << '\n';
    return 1;
}
