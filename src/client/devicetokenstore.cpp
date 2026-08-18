module;

#include <fstream>
#include <stdexcept>
#include <string>

module will.client.devicetokenstore;

import will.domain.device_token;


namespace will {


std::string DeviceTokenStore::load_or_create(const std::string& path)
{
    {
        std::ifstream in(path);
        if (in) {
            std::string token;
            in >> token;
            if (const auto parsed = domain::DeviceToken::parse(token))
                return parsed->value().value;
        }
    }

    const domain::AuthToken token = domain::DeviceToken::generate();

    std::ofstream out(path, std::ios::trunc);
    if (!out)
        throw std::runtime_error("DeviceTokenStore: failed to write " + path);

    out << token.value;
    if (!out)
        throw std::runtime_error("DeviceTokenStore: failed to persist device token");

    return token.value;
}


} // namespace will
