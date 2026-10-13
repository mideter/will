#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "serverconfigvalidator.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <unistd.h>
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


TEST_CASE("ServerConfig takes the databases only from an existing directory")
{
	{
		ServerConfig config;
		config.db_path = "/nonexistent-will-directory";
		require_throws_field(config, "db_path");
	}
	{
		// A path to a file, as the database path once was, is refused rather than
		// taken for an empty world.
		const std::string file = "/tmp/will-config-test-" + std::to_string(::getpid()) + ".db";
		std::ofstream{file} << "";
		ServerConfig config;
		config.db_path = file;
		require_throws_field(config, "db_path");
		std::filesystem::remove(file);
	}
	{
		ServerConfig config;
		config.db_path = "/tmp";
		CHECK_NOTHROW(ServerConfigValidator::validate(config));
	}
}


TEST_CASE("ServerConfig takes TLS from an existing certificate and key together, on a port of its own")
{
	const std::filesystem::path directory = "/tmp/will-config-tls-test-" + std::to_string(getpid());
	std::filesystem::create_directories(directory);
	const std::string cert = (directory / "server.crt").string();
	const std::string key = (directory / "server.key").string();
	std::ofstream{cert} << "cert";
	std::ofstream{key} << "key";

	ServerConfig config;
	config.tls_cert = cert;
	config.tls_key = key;
	CHECK(config.tls());
	CHECK_NOTHROW(ServerConfigValidator::validate(config));

	// The certificate without its key, or a file that is not there, is not taken.
	ServerConfig without_key = config;
	without_key.tls_key.clear();
	require_throws_field(without_key, "tls_key");
	ServerConfig missing = config;
	missing.tls_cert = (directory / "absent.crt").string();
	require_throws_field(missing, "tls_cert");

	// TLS listens beside the open port, not on it.
	ServerConfig same_port = config;
	same_port.tls_port = same_port.listen_port;
	require_throws_field(same_port, "tls_port");

	// The open port may be closed only when TLS is there to listen.
	ServerConfig tls_only = config;
	tls_only.plain = false;
	tls_only.tls_port = tls_only.listen_port;
	CHECK_NOTHROW(ServerConfigValidator::validate(tls_only));
	ServerConfig nothing;
	nothing.plain = false;
	require_throws_field(nothing, "plain");

	std::filesystem::remove_all(directory);
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
