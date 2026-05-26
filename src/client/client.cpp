#include <iostream>

#include "chatsession.h"
#include "clientconfigparser.h"
#include "willclient.h"


int main(int argc, char* argv[])
try {
    const will::ClientConfigParser cli(argc, argv);
    const will::ClientConfig& cfg = cli.client_config();

    will::WillClient client(cfg);

    will::ChatSession chat_session(client);
    chat_session.run();

    return 0;
}
catch (const std::exception& e) {
    std::cerr << "Client error: " << e.what() << '\n';
    return 1;
}
