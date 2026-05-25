#pragma once

#include "loadclientsconfig.h"

#include <CLI/CLI.hpp>

#include <iosfwd>


namespace will {
namespace cli {


class LoadClientsCliApp {
public:
    explicit LoadClientsCliApp(const LoadClientsConfig& defaults = LoadClientsConfig{});

    void print_help(std::ostream& os) const;

    [[noreturn]] void exit_on_help(const CLI::CallForHelp& error) const;

    [[noreturn]] void exit_on_parse_error(const CLI::ParseError& error) const;

    LoadClientsConfig parse(int argc, char* argv[]);

private:
    void apply_to(LoadClientsConfig& config) const;

    CLI::App app_;

    std::string host_;
    int port_;
    std::size_t clients_;
    std::size_t messages_per_client_;
    int hold_seconds_;
};


} // namespace cli
} // namespace will
