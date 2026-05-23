#pragma once

#include "clioption.h"

#include <iosfwd>
#include <string_view>


namespace will {


class PortCliOption final : public CliOption {
public:
    PortCliOption();

    static const PortCliOption& instance();

    void print_usage(std::ostream& os) const override;
};


class IoThreadsCliOption final : public CliOption {
public:
    IoThreadsCliOption();

    static const IoThreadsCliOption& instance();

    void print_usage(std::ostream& os) const override;
};


class ListenBacklogCliOption final : public CliOption {
public:
    ListenBacklogCliOption();

    static const ListenBacklogCliOption& instance();

    void print_usage(std::ostream& os) const override;
};


class MaxClientsCliOption final : public CliOption {
public:
    MaxClientsCliOption();

    static const MaxClientsCliOption& instance();

    void print_usage(std::ostream& os) const override;
};


class MaxOutboundQueueCliOption final : public CliOption {
public:
    MaxOutboundQueueCliOption();

    static const MaxOutboundQueueCliOption& instance();

    void print_usage(std::ostream& os) const override;
};


class HelpCliOption final : public CliOption {
public:
    HelpCliOption();

    static const HelpCliOption& instance();

    bool matches(std::string_view text) const override;

    void print_usage(std::ostream& os) const override;
};


} // namespace will
