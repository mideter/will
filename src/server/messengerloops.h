#pragma once

#include <memory>
#include <mutex>
#include <vector>


namespace will {


class Client;
class ClientHub;


/** Per-client read loop and frame broadcast for the messenger server. */
class MessengerLoops {
public:
	static void broadcast_from_sender(ClientHub& hub, Client& sender, const std::vector<char>& payload);
	static void reader_main(ClientHub& hub, std::shared_ptr<Client> client, int sig_slot);

private:
	static std::mutex frame_log_mutex_;
};


} // namespace will
