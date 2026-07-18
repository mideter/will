#include "support/user_name.h"

#include <random>


namespace will::domain {


namespace {


constexpr char Alphabet[] = "abcdefghijklmnopqrstuvwxyz0123456789";
constexpr std::size_t AlphabetSize = sizeof(Alphabet) - 1;


} // namespace


std::string UserName::generate()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<std::size_t> dist(0, AlphabetSize - 1);

    std::string name(Length, '\0');
    for (char& ch : name)
        ch = Alphabet[dist(gen)];
    return name;
}


bool UserName::is_valid(const std::string_view name) noexcept
{
    if (name.size() != Length)
        return false;

    for (const char c : name) {
        const bool ok = (c >= 'a' && c <= 'z') || (c >= '0' && c <= '9');
        if (!ok)
            return false;
    }
    return true;
}


} // namespace will::domain
