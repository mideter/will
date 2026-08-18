module;

#include <cstddef>
#include <string>
#include <string_view>

export module will.domain.user_name;

export namespace will::domain {


/** Random public user label: Latin letters a-z and digits, fixed length. */
class UserName {
public:
    static constexpr std::size_t Length = 8;

    static std::string generate();
    static bool is_valid(std::string_view name) noexcept;
};


} // namespace will::domain
