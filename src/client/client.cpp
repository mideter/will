#include <iostream>

#include "chatsession.h"
#include "willclient.h"


int main()
try {
    const will::ServerAddress server_address = will::ServerAddress::Local;
    
    will::WillClient client;
    client.connect(server_address);

    will::ChatSession chat_session(client);
    chat_session.run();

    return 0;
} 
catch (const std::exception& e) {
    std::cerr << e.what() << '\n';
    return 1;
}
