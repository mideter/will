#pragma once

#include <memory>
#include <mutex>
#include <vector>

#include "client.h"


namespace will {


class ClientHub;


/** Per-client session in a worker thread: read frames and broadcast to peers. */
class MessengerLoops {
public:
    static void broadcast_from_sender(ClientHub& hub, Client& sender, const std::vector<char>& payload);
    static void run_client_session(ClientHub& hub, std::shared_ptr<Client> client);

private:
    static void gather_recipients_excluding_sender(const ClientHub& hub,
                                                   const Client& sender,
                                                   std::vector<std::shared_ptr<Client>>& recipients_out);

    static void log_frame_no_other_peers(const Client& sender, const std::vector<char>& payload);

    /** @return false if peer disconnected before the next frame. */
    static bool recv_client_message(Client& client, std::vector<char>& payload_out);

    static void send_receipt_ack_to_sender(Client& sender);

    static void send_payload_to_recipients(ClientHub& hub,
                                           const std::vector<std::shared_ptr<Client>>& recipients,
                                           const std::vector<char>& payload);
    
    static void log_frame_broadcast_summary(const Client& sender,
                                            std::size_t recipient_count,
                                            const std::vector<char>& payload);

    static std::mutex frame_log_mutex_;
};


} // namespace will
