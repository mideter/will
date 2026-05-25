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
        config.io_threads = 0;
        assert_throws_field(config, "io_threads");
    }

    {
        ServerConfig config;
        config.listen_backlog = 0;
        assert_throws_field(config, "listen_backlog");
    }

    {
        ServerConfig config;
        config.max_connections = 0;
        assert_throws_field(config, "max_connections");
    }

    {
        ServerConfig config;
        config.max_outbound_queue_bytes = 0;
        assert_throws_field(config, "max_outbound_queue_bytes");
    }

    {
        ServerConfig config;
        config.listen_port = 9000;
        config.io_threads = 2;
        const ServerConfig accepted = ServerConfigValidator::accept(std::move(config));
        assert(accepted.listen_port == 9000);
        assert(accepted.io_threads == 2);
    }

    return EXIT_SUCCESS;
}
