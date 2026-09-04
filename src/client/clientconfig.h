#pragma once

#include <cstdint>
#include <string>
#include <string_view>

// ColorMode is defined below; cstdint provides the underlying type.


namespace will {


enum class ColorMode : std::uint8_t {
	Auto,
	Always,
	Never,
};


/** Client configuration (no invariants; validated by ClientConfigValidator). */
struct ClientConfig {
	static constexpr int MinPort = 1;
	static constexpr int MaxPort = 65535;
	static constexpr std::uint16_t DefaultPort = 7770;
	static constexpr std::string_view NovosibirskHost = "83.217.202.145";
	static constexpr std::string_view DefaultHost = "127.0.0.1";
	static constexpr std::string_view DefaultDeviceTokenPath = "will.device_token";
	static constexpr bool DefaultQuietReceipts = false;
	static constexpr std::uint32_t DefaultHistoryLimit = 50;
	static constexpr ColorMode DefaultColor = ColorMode::Auto;

	std::string host = std::string{DefaultHost};
	std::uint16_t port = DefaultPort;
	std::string device_token_path = std::string{DefaultDeviceTokenPath};
	bool quiet_receipts = DefaultQuietReceipts;
	std::uint32_t history_limit = DefaultHistoryLimit;
	ColorMode color = DefaultColor;
};


} // namespace will
