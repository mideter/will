#pragma once

#include "clioption.h"

#include <iosfwd>
#include <string_view>


namespace will {


class PortCliOption final : public CliOption {
public:
    PortCliOption();

    [[nodiscard]] static const PortCliOption& instance();

    void print_usage(std::ostream& os) const override;
};


class IoThreadsCliOption final : public CliOption {
public:
    IoThreadsCliOption();

    [[nodiscard]] static const IoThreadsCliOption& instance();

    void print_usage(std::ostream& os) const override;
};


class ListenBacklogCliOption final : public CliOption {
public:
    ListenBacklogCliOption();

    [[nodiscard]] static const ListenBacklogCliOption& instance();

    void print_usage(std::ostream& os) const override;
};


class MaxClientsCliOption final : public CliOption {
public:
    MaxClientsCliOption();

    [[nodiscard]] static const MaxClientsCliOption& instance();

    void print_usage(std::ostream& os) const override;
};


class MaxOutboundQueueCliOption final : public CliOption {
public:
    MaxOutboundQueueCliOption();

    [[nodiscard]] static const MaxOutboundQueueCliOption& instance();

    void print_usage(std::ostream& os) const override;
};


class HelpCliOption final : public CliOption {
public:
    HelpCliOption();

    [[nodiscard]] static const HelpCliOption& instance();

    [[nodiscard]] bool matches(std::string_view text) const override;

    void print_usage(std::ostream& os) const override;
};


} // namespace will
