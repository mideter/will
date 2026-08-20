module;

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <vector>

export module will.server.protocol_logic;

export namespace will {


using BroadcastChat = void (*)(void* registry, std::uint64_t except_session, std::string_view name,
                               std::string_view body);


struct BindTokenResult {
    enum class Status { Ok, InvalidToken };
    Status status = Status::InvalidToken;
    std::optional<std::uint64_t> displaced_session;
};


struct HistoryItemDto {
    std::uint64_t message_id = 0;
    bool is_mine = false;
    std::string name;
    std::string body;
};


struct HistoryResult {
    enum class Status { Ok, Invalid };
    Status status = Status::Invalid;
    std::vector<HistoryItemDto> items;
};


class SessionRuntime {
public:
    SessionRuntime(void* messages, void* users, void* accounts, void* registry, BroadcastChat broadcast);
    ~SessionRuntime();

    SessionRuntime(const SessionRuntime&) = delete;
    SessionRuntime& operator=(const SessionRuntime&) = delete;

    [[nodiscard]] bool has_account(std::uint64_t session_id) const;

    BindTokenResult bind_token(std::uint64_t session_id, std::string_view token);
    void send_chat(std::uint64_t session_id, std::string_view body);
    HistoryResult fetch_history(std::uint64_t session_id, std::uint32_t limit);

private:
    struct Impl;
    std::unique_ptr<Impl> impl_;
};


} // namespace will
