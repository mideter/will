#include <iostream>

#include "chatsession.h"
#include "clientconfigparser.h"
#include "willclient.h"


int main(int argc, char* argv[])
try {
    const will::ClientConfigParser cli(argc, argv);
    will::WillClient client(cli.client_config());
    client.connect();
    client.authenticate(client.config().login, client.config().password);

    will::ChatSession chat_session(client);
    chat_session.run();

    return 0;
}
catch (const std::exception& e) {
    std::cerr << "Client error: " << e.what() << '\n';
    return 1;
}
