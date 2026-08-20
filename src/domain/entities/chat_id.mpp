module;

#include <compare>
#include <cstdint>

export module will.domain.chat_id;

export namespace will::domain {


struct ChatId {
    std::uint64_t value = 0;

    ChatId() noexcept;
    constexpr explicit ChatId(std::uint64_t v) noexcept : value(v) {}

    static constexpr ChatId global() noexcept { return ChatId{0}; }

    constexpr auto operator<=>(const ChatId&) const noexcept = default;
    constexpr bool operator==(const ChatId&) const noexcept = default;
};


ChatId::ChatId() noexcept = default;


} // namespace will::domain
