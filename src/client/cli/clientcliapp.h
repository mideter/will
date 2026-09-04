#pragma once

#include "clientconfig.h"

#include <CLI/CLI.hpp>

#include <iosfwd>


namespace will {


class ClientCliApp {
public:
	ClientCliApp();

	void print_help(std::ostream& os) const;

	[[noreturn]] void exit_on_help(const CLI::CallForHelp& error) const;

	[[noreturn]] void exit_on_parse_error(const CLI::ParseError& error) const;

	ClientConfig parse(int argc, char* argv[]);

private:
	CLI::App app_;
	ClientConfig config_;
};


} // namespace will
