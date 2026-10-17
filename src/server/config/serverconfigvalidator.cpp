#include "serverconfigvalidator.h"

#include <filesystem>
#include <format>
#include <system_error>


namespace will {


ServerConfigError::ServerConfigError(const std::string_view field, const std::string_view reason)
	: std::invalid_argument(std::format("{} {}", field, reason))
{}


void ServerConfigValidator::require(const std::string_view field, const std::optional<std::string_view> reason)
{
	if (reason)
		throw ServerConfigError(field, *reason);
}


std::optional<std::string_view> ServerConfigValidator::listen_port_reason(const std::uint16_t port)
{
	if (port < ServerConfig::MinListenPort || port > ServerConfig::MaxListenPort)
		return "must be between 1 and 65535";

	return std::nullopt;
}


std::optional<std::string_view> ServerConfigValidator::db_path_reason(const std::string_view db_path)
{
	if (db_path.empty())
		return "must not be empty";
	// The databases live in an existing directory: a missing one is not made, lest
	// a mistaken path start an empty world.
	std::error_code error;
	if (!std::filesystem::is_directory(std::filesystem::path{db_path}, error))
		return "must be an existing directory";

	return std::nullopt;
}


std::optional<std::string_view> ServerConfigValidator::pem_file_reason(const std::string_view path)
{
	if (path.empty())
		return "must be given together with the other TLS file";
	std::error_code error;
	if (!std::filesystem::is_regular_file(std::filesystem::path{path}, error))
		return "must be an existing file";

	return std::nullopt;
}


template<typename T>
std::optional<std::string_view> ServerConfigValidator::positive_reason(const T value)
{
	if (value < 1)
		return "must be at least 1";

	return std::nullopt;
}


void ServerConfigValidator::validate(const ServerConfig& config)
{
	require("listen_port", listen_port_reason(config.listen_port));
	require("max_connections", positive_reason(config.max_connections));
	require("db_path", db_path_reason(config.db_path));
	require("keepalive_interval_seconds", positive_reason(config.keepalive_interval_seconds));
	require("keepalive_timeout_seconds", positive_reason(config.keepalive_timeout_seconds));
	if (config.tls()) {
		require("tls_cert", pem_file_reason(config.tls_cert));
		require("tls_key", pem_file_reason(config.tls_key));
		require("tls_port", listen_port_reason(config.tls_port));
		if (config.plain && config.tls_port == config.listen_port)
			throw ServerConfigError("tls_port", "must differ from the open port");
	} else if (!config.plain) {
		// Closing the open port leaves nothing to listen on without TLS.
		throw ServerConfigError("plain", "may be closed only when TLS is given");
	}
}


ServerConfig ServerConfigValidator::accept(ServerConfig config)
{
	validate(config);
	return config;
}


} // namespace will
