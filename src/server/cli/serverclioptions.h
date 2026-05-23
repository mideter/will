#pragma once

#include "clioption.h"
#include "serverconfig.h"

#include <iosfwd>
#include <string_view>


namespace will {


class PortCliOption final : public CliOption<ServerConfig, PortCliOption> {
public:
    [[nodiscard]] bool matches(std::string_view text) const override;
    [[nodiscard]] std::string_view primary_flag() const override;

    void print_usage(std::ostream& os) const override;
    void apply(CliParserContext<ServerConfig>& context) const override;
};


class IoThreadsCliOption final : public CliOption<ServerConfig, IoThreadsCliOption> {
public:
    [[nodiscard]] bool matches(std::string_view text) const override;
    [[nodiscard]] std::string_view primary_flag() const override;

    void print_usage(std::ostream& os) const override;
    void apply(CliParserContext<ServerConfig>& context) const override;
};


class ListenBacklogCliOption final : public CliOption<ServerConfig, ListenBacklogCliOption> {
public:
    [[nodiscard]] bool matches(std::string_view text) const override;
    [[nodiscard]] std::string_view primary_flag() const override;

    void print_usage(std::ostream& os) const override;
    void apply(CliParserContext<ServerConfig>& context) const override;
};


class MaxClientsCliOption final : public CliOption<ServerConfig, MaxClientsCliOption> {
public:
    [[nodiscard]] bool matches(std::string_view text) const override;
    [[nodiscard]] std::string_view primary_flag() const override;

    void print_usage(std::ostream& os) const override;
    void apply(CliParserContext<ServerConfig>& context) const override;
};


class MaxOutboundQueueCliOption final : public CliOption<ServerConfig, MaxOutboundQueueCliOption> {
public:
    [[nodiscard]] bool matches(std::string_view text) const override;
    [[nodiscard]] std::string_view primary_flag() const override;

    void print_usage(std::ostream& os) const override;
    void apply(CliParserContext<ServerConfig>& context) const override;
};


class HelpCliOption final : public CliOption<ServerConfig, HelpCliOption> {
public:
    [[nodiscard]] bool matches(std::string_view text) const override;
    [[nodiscard]] std::string_view primary_flag() const override;

    void print_usage(std::ostream& os) const override;
    void apply(CliParserContext<ServerConfig>& context) const override;
};


} // namespace will
