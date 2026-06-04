#include "clientconfigvalidator.h"

#include <cassert>
#include <cstdlib>
#include <string>


namespace {


void assert_throws_field(const will::ClientConfig& config, const char* field)
{
    try {
        will::ClientConfigValidator::validate(config);
        assert(false && "expected ClientConfigError");
    } catch (const will::ClientConfigError& error) {
        const std::string message = error.what();
        assert(message.find(field) != std::string::npos);
    }
}


} // namespace


int main()
{
    using namespace will;

    ClientConfigValidator::validate({});

    {
        ClientConfig config;
        config.host = "";
        assert_throws_field(config, "host");
    }

    {
        ClientConfig config;
        config.host = "not-an-ip";
        assert_throws_field(config, "host");
    }

    {
        ClientConfig config;
        config.port = 0;
        assert_throws_field(config, "port");
    }

    {
        ClientConfig config;
        config.login = "";
        assert_throws_field(config, "login");
    }

    {
        ClientConfig config;
        config.password = "";
        assert_throws_field(config, "password");
    }

    {
        ClientConfig config;
        config.host = "192.168.1.10";
        config.port = 9000;
        const ClientConfig accepted = ClientConfigValidator::accept(std::move(config));
        assert(accepted.host == "192.168.1.10");
        assert(accepted.port == 9000);
    }

    return EXIT_SUCCESS;
}
