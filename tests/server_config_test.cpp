#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "serverconfigvalidator.h"

#include <string>
#include <utility>


using namespace will;


namespace {


void require_throws_field(const ServerConfig& config, const char* field)
{
	try {
		ServerConfigValidator::validate(config);
		FAIL_CHECK("expected ServerConfigError");
	} catch (const ServerConfigError& error) {
		CHECK(std::string(error.what()).find(field) != std::string::npos);
	}
}


} // namespace


TEST_CASE("ServerConfig defaults are valid")
{
	CHECK_NOTHROW(ServerConfigValidator::validate({}));
}


TEST_CASE("ServerConfig rejects zero fields")
{
	{
		ServerConfig config;
		config.listen_port = 0;
		require_throws_field(config, "listen_port");
	}
	{
		ServerConfig config;
		config.max_connections = 0;
		require_throws_field(config, "max_connections");
	}
	{
		ServerConfig config;
		config.keepalive_interval_seconds = 0;
		require_throws_field(config, "keepalive_interval_seconds");
	}
	{
		ServerConfig config;
		config.keepalive_timeout_seconds = 0;
		require_throws_field(config, "keepalive_timeout_seconds");
	}
}


TEST_CASE("ServerConfigValidator::accept returns validated config")
{
	ServerConfig config;
	config.listen_port = 9000;
	config.max_connections = 2;
	const ServerConfig accepted = ServerConfigValidator::accept(std::move(config));
	CHECK(accepted.listen_port == 9000);
	CHECK(accepted.max_connections == 2);
}
