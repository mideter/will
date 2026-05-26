#pragma once

#include <cstdint>
#include <mutex>
#include <string>
#include <string_view>
#include <vector>


struct sqlite3;


namespace will {


struct StoredMessage {
    std::uint64_t id = 0;
    std::string body;
    std::string sender_ip;
    std::int64_t created_at_ms = 0;
};


class MessageStore {
public:
    explicit MessageStore(std::string db_path);
    ~MessageStore();

    MessageStore(const MessageStore&) = delete;
    MessageStore& operator=(const MessageStore&) = delete;

    std::uint64_t insert_message(std::string_view body, std::string_view sender_ip,
                                 std::int64_t created_at_ms);

    std::vector<StoredMessage> load_last(std::uint32_t limit);

private:
    void open_database();
    void init_schema();

    std::string db_path_;
    sqlite3* db_ = nullptr;
    std::mutex mutex_;
};


} // namespace will
