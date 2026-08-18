export module will.domain.chat;

export import will.domain.chat_id;

export namespace will::domain {


/// Chat room; single global chat is represented by ChatId::global().
struct Chat {
    ChatId id = ChatId::global();

    static constexpr Chat global() noexcept { return Chat{}; }
};


} // namespace will::domain
