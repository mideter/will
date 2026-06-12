#pragma once

#include <cstdint>
#include <string>
#include <string_view>


namespace will {


/** Client configuration (no invariants; validated by ClientConfigValidator). */
struct ClientConfig {
    static constexpr int MinPort = 1;
    static constexpr int MaxPort = 65535;
    static constexpr std::uint16_t DefaultPort = 7770;
    static constexpr std::string_view NovosibirskHost = "83.217.202.145";
    static constexpr std::string_view DefaultHost = "127.0.0.1";
    static constexpr std::string_view DefaultPhone = "+15551234567";
    static constexpr bool DefaultQuietReceipts = false;
    static constexpr std::uint32_t DefaultHistoryLimit = 50;

    std::string host = std::string{DefaultHost};
    std::uint16_t port = DefaultPort;
    std::string phone = std::string{DefaultPhone};
    std::string otp;
    bool quiet_receipts = DefaultQuietReceipts;
    std::uint32_t history_limit = DefaultHistoryLimit;
};


} // namespace will
