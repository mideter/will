module;

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>

export module will.client.clientconfigvalidator;

export import will.client.clientconfig;

export namespace will {


class ClientConfigError : public std::invalid_argument {
public:
    ClientConfigError(std::string_view field, std::string_view reason);
};


class ClientConfigValidator {
public:
    static void validate(const ClientConfig& config);
    static ClientConfig accept(ClientConfig config);

private:
    static void require(std::string_view field, std::optional<std::string_view> reason);
    static std::optional<std::string_view> host_reason(const std::string& host);
    static std::optional<std::string_view> port_reason(decltype(ClientConfig::port) port);
    static std::optional<std::string_view> device_token_path_reason(const std::string& path);
};


} // namespace will
