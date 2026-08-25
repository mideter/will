#include "serverconfigvalidator.h"

#include <cassert>
#include <cstdlib>
#include <string>


namespace {


void assert_throws_field(const will::ServerConfig& config, const char* field)
{
    try {
        will::ServerConfigValidator::validate(config);
        assert(false && "expected ServerConfigError");
    } catch (const will::ServerConfigError& error) {
        const std::string message = error.what();
        assert(message.find(field) != std::string::npos);
    }
}


} // namespace


int main()
{
    using namespace will;

    ServerConfigValidator::validate({});

    {
        ServerConfig config;
        config.listen_port = 0;
        assert_throws_field(config, "listen_port");
    }

    {
        ServerConfig config;
        config.max_connections = 0;
        assert_throws_field(config, "max_connections");
    }

    {
        ServerConfig config;
        config.keepalive_interval_seconds = 0;
        assert_throws_field(config, "keepalive_interval_seconds");
    }

    {
        ServerConfig config;
        config.keepalive_timeout_seconds = 0;
        assert_throws_field(config, "keepalive_timeout_seconds");
    }

    {
        ServerConfig config;
        config.listen_port = 9000;
        config.max_connections = 2;
        const ServerConfig accepted = ServerConfigValidator::accept(std::move(config));
        assert(accepted.listen_port == 9000);
        assert(accepted.max_connections == 2);
    }

    return EXIT_SUCCESS;
}
