#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "clientconfigvalidator.h"

#include <string>
#include <utility>


using namespace will;


namespace {


void require_throws_field(const ClientConfig& config, const char* field)
{
	try {
		ClientConfigValidator::validate(config);
		FAIL_CHECK("expected ClientConfigError");
	} catch (const ClientConfigError& error) {
		CHECK(std::string(error.what()).find(field) != std::string::npos);
	}
}


} // namespace


TEST_CASE("ClientConfig defaults are valid")
{
	CHECK_NOTHROW(ClientConfigValidator::validate({}));
}


TEST_CASE("ClientConfig rejects invalid fields")
{
	{
		ClientConfig config;
		config.host = "";
		require_throws_field(config, "host");
	}
	{
		ClientConfig config;
		config.host = "not-an-ip";
		require_throws_field(config, "host");
	}
	{
		ClientConfig config;
		config.port = 0;
		require_throws_field(config, "port");
	}
	{
		ClientConfig config;
		config.device_token_path = "";
		require_throws_field(config, "device_token_path");
	}
}


TEST_CASE("ClientConfigValidator::accept returns validated config")
{
	ClientConfig config;
	config.host = "192.168.1.10";
	config.port = 9000;
	config.device_token_path = "/tmp/will.device_token";
	const ClientConfig accepted = ClientConfigValidator::accept(std::move(config));
	CHECK(accepted.host == "192.168.1.10");
	CHECK(accepted.port == 9000);
	CHECK(accepted.device_token_path == "/tmp/will.device_token");
}
