module;

#include <optional>
#include <string_view>

export module will.domain.user_repository;

export import will.domain.user;
export import will.domain.user_id;

export namespace will::domain {


class UserRepository {
public:
    virtual ~UserRepository() = default;

    virtual std::optional<User> find_by_device_token(std::string_view device_token) = 0;
    virtual User create_user(std::string_view device_token, std::string_view name) = 0;
    virtual void set_name(UserId id, std::string_view name) = 0;
};


} // namespace will::domain
