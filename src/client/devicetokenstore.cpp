#include "devicetokenstore.h"

#include <fstream>
#include <stdexcept>


namespace will {


domain::DeviceToken DeviceTokenStore::load_or_create(const std::string& path)
{
    {
        std::ifstream in(path);
        if (in) {
            std::string token;
            in >> token;
            if (const auto parsed = domain::DeviceToken::parse(token))
                return *parsed;
        }
    }

    const domain::DeviceToken token = domain::DeviceToken::generate();

    std::ofstream out(path, std::ios::trunc);
    if (!out)
        throw std::runtime_error("DeviceTokenStore: failed to write " + path);

    out << token.text();
    if (!out)
        throw std::runtime_error("DeviceTokenStore: failed to persist device token");

    return token;
}


} // namespace will
