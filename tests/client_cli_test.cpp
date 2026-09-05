#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "clientcliapp.h"
#include "clientconfig.h"

#include <CLI/CLI.hpp>

#include <sstream>
#include <string>
#include <vector>


namespace {


struct Argv {
	std::vector<std::string> storage;
	std::vector<char*> ptrs;

	explicit Argv(std::initializer_list<const char*> args)
	{
		storage.reserve(args.size());
		for (const char* arg : args)
			storage.emplace_back(arg);

		ptrs.reserve(storage.size());
		for (std::string& arg : storage)
			ptrs.push_back(arg.data());

		ptrs.push_back(nullptr);
	}

	int argc() const { return static_cast<int>(storage.size()); }

	char** argv() { return ptrs.data(); }
};


} // namespace


TEST_CASE("will-client help output")
{
	std::ostringstream out;
	will::ClientCliApp{}.print_help(out);
	const std::string help = out.str();

	CHECK(help.find("Usage: will-client") != std::string::npos);
	CHECK(help.find("--host") != std::string::npos);
	CHECK(help.find(will::ClientConfig::NovosibirskHost) != std::string::npos);
	CHECK(help.find("--port") != std::string::npos);
	CHECK(help.find("--quiet") != std::string::npos);
	CHECK(help.find("--device-token-path") != std::string::npos);
	CHECK(help.find("--no-history") != std::string::npos);
	CHECK(help.find("--color") != std::string::npos);
	CHECK(help.find("-h, --help") != std::string::npos);
}


TEST_CASE("will-client parse errors")
{
	will::ClientCliApp cli;
	{
		Argv argv{"will-client", "--unknown"};
		CHECK_THROWS_AS(cli.parse(argv.argc(), argv.argv()), CLI::ParseError);
	}
	{
		Argv argv{"will-client", "--port"};
		CHECK_THROWS_AS(cli.parse(argv.argc(), argv.argv()), CLI::ParseError);
	}
	{
		Argv argv{"will-client", "--color", "rainbow"};
		CHECK_THROWS_AS(cli.parse(argv.argc(), argv.argv()), CLI::ParseError);
	}
}


TEST_CASE("will-client help requested")
{
	will::ClientCliApp cli;
	for (std::initializer_list<const char*> args : {
			 std::initializer_list<const char*>{"will-client", "--help"},
			 {"will-client", "-h"},
			 {"will-client", "--help", "--port", "8080"},
			 {"will-client", "--port", "8080", "--help"},
		 }) {
		Argv argv{args};
		CHECK_THROWS_AS(cli.parse(argv.argc(), argv.argv()), CLI::CallForHelp);
	}
}


TEST_CASE("will-client defaults")
{
	Argv args{"will-client"};
	const will::ClientConfig config = will::ClientCliApp{}.parse(args.argc(), args.argv());

	CHECK(config.host == will::ClientConfig::DefaultHost);
	CHECK(config.port == will::ClientConfig::DefaultPort);
	CHECK(config.device_token_path == will::ClientConfig::DefaultDeviceTokenPath);
	CHECK(config.quiet_receipts == will::ClientConfig::DefaultQuietReceipts);
	CHECK(config.history_limit == will::ClientConfig::DefaultHistoryLimit);
	CHECK(config.color == will::ClientConfig::DefaultColor);
}


TEST_CASE("will-client --no-history")
{
	Argv args{"will-client", "--no-history"};
	const will::ClientConfig config = will::ClientCliApp{}.parse(args.argc(), args.argv());
	CHECK(config.history_limit == 0);
}


TEST_CASE("will-client all options")
{
	Argv args{"will-client", "--host", "192.168.1.10", "--port", "9000", "--device-token-path",
			  "/tmp/token", "--quiet", "--history", "25", "--color", "never"};
	const will::ClientConfig config = will::ClientCliApp{}.parse(args.argc(), args.argv());

	CHECK(config.host == "192.168.1.10");
	CHECK(config.port == 9000);
	CHECK(config.device_token_path == "/tmp/token");
	CHECK(config.quiet_receipts);
	CHECK(config.history_limit == 25u);
	CHECK(config.color == will::ColorMode::Never);
}


TEST_CASE("will-client --color always")
{
	Argv args{"will-client", "--color", "always"};
	const will::ClientConfig config = will::ClientCliApp{}.parse(args.argc(), args.argv());
	CHECK(config.color == will::ColorMode::Always);
}


TEST_CASE("will-client --port 0 parses")
{
	Argv args{"will-client", "--port", "0"};
	const will::ClientConfig config = will::ClientCliApp{}.parse(args.argc(), args.argv());
	CHECK(config.port == 0);
}
