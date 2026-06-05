#pragma once

#include "clientconfig.h"

#include <CLI/CLI.hpp>

#include <cstdint>
#include <iosfwd>
#include <string>


namespace will {


class ClientCliApp {
public:
    explicit ClientCliApp(const ClientConfig& defaults = ClientConfig{});

    void print_help(std::ostream& os) const;

    [[noreturn]] void exit_on_help(const CLI::CallForHelp& error) const;

    [[noreturn]] void exit_on_parse_error(const CLI::ParseError& error) const;

    ClientConfig parse(int argc, char* argv[]);

private:
    void apply_to(ClientConfig& config) const;

    CLI::App app_;

    std::string host_;
    int port_;
    std::string login_;
    std::string password_;
    bool quiet_receipts_;
    std::uint32_t history_limit_;
};


} // namespace will
