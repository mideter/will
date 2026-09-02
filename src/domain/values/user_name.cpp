#include "user_name.h"

#include <random>


namespace will::domain {


namespace {


constexpr char Alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789";
constexpr std::size_t AlphabetSize = sizeof(Alphabet) - 1;


} // namespace


std::optional<UserName> UserName::parse(const std::string_view input)
{
    if (input.size() != Length)
        return std::nullopt;

    for (const char c : input) {
        const bool ok = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
        if (!ok)
            return std::nullopt;
    }

    return UserName{std::string(input)};
}


UserName UserName::generate()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> dist(0, AlphabetSize - 1);

    std::string name(Length, '\0');
    for (char& ch : name)
        ch = Alphabet[dist(gen)];
    return UserName{std::move(name)};
}


UserName::UserName(std::string value) : value_(std::move(value)) {}


} // namespace will::domain
