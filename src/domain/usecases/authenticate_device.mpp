module;

#include <string_view>
#include <variant>

export module will.domain.authenticate_device;

export import will.domain.account;
export import will.domain.auth_error;
export import will.domain.auth_token;
export import will.domain.user_repository;

export namespace will::domain {


struct AuthenticateDeviceInput {
    std::string_view device_token_raw;
    Timestamp now{};
};


struct AuthenticateDeviceSuccess {
    Account account;
};


class AuthenticateDevice {
public:
    explicit AuthenticateDevice(UserRepository& users);

    [[nodiscard]] std::variant<AuthenticateDeviceSuccess, AuthError> execute(const AuthenticateDeviceInput& input);

private:
    UserRepository& users_;
};


} // namespace will::domain
