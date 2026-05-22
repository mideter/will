#include <iostream>

#include "chatsession.h"
#include "hostaddress.h"
#include "willclient.h"


int main()
try {
    const will::HostAddress server_address{"127.0.0.1", 7770};
    
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
