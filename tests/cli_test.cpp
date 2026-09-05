#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest/doctest.h>

#include "servercliapp.h"
#include "serverconfig.h"

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


TEST_CASE("will-server help output")
{
	std::ostringstream out;
	will::ServerCliApp{}.print_help(out);
	const std::string help = out.str();

	CHECK(help.find("Usage: will-server") != std::string::npos);
	CHECK(help.find("--port") != std::string::npos);
	CHECK(help.find("--keepalive-interval") != std::string::npos);
	CHECK(help.find("--keepalive-timeout") != std::string::npos);
	CHECK(help.find("-h, --help") != std::string::npos);
	CHECK(help.find("OPTIONS:") == std::string::npos);
	CHECK(help.find(" INT") == std::string::npos);
	CHECK(help.find(" UINT") == std::string::npos);
	CHECK(help.find("[OPTIONS]") == std::string::npos);
}


TEST_CASE("will-server parse errors")
{
	will::ServerCliApp cli;
	{
		Argv argv{"will-server", "--unknown"};
		CHECK_THROWS_AS(cli.parse(argv.argc(), argv.argv()), CLI::ParseError);
	}
	{
		Argv argv{"will-server", "--port"};
		CHECK_THROWS_AS(cli.parse(argv.argc(), argv.argv()), CLI::ParseError);
	}
	{
		Argv argv{"will-server", "--max-clients", "abc"};
		CHECK_THROWS_AS(cli.parse(argv.argc(), argv.argv()), CLI::ParseError);
	}
}


TEST_CASE("will-server help requested")
{
	will::ServerCliApp cli;
	for (std::initializer_list<const char*> args : {
			 std::initializer_list<const char*>{"will-server", "--help"},
			 {"will-server", "-h"},
			 {"will-server", "--help", "--port", "8080"},
			 {"will-server", "--port", "8080", "--help"},
		 }) {
		Argv argv{args};
		CHECK_THROWS_AS(cli.parse(argv.argc(), argv.argv()), CLI::CallForHelp);
	}
}


TEST_CASE("will-server defaults")
{
	Argv args{"will-server"};
	const will::ServerConfig config = will::ServerCliApp{}.parse(args.argc(), args.argv());

	CHECK(config.listen_port == will::ServerConfig::DefaultListenPort);
	CHECK(config.max_connections == will::ServerConfig::DefaultMaxConnections);
	CHECK(config.db_path == will::ServerConfig::DefaultDbPath);
	CHECK(config.keepalive_interval_seconds == will::ServerConfig::DefaultKeepaliveIntervalSeconds);
	CHECK(config.keepalive_timeout_seconds == will::ServerConfig::DefaultKeepaliveTimeoutSeconds);
}


TEST_CASE("will-server all options")
{
	Argv args{"will-server",
			  "--port",
			  "9000",
			  "--max-clients",
			  "128",
			  "--db-path",
			  "/tmp/custom.db",
			  "--keepalive-interval",
			  "15",
			  "--keepalive-timeout",
			  "5"};
	const will::ServerConfig config = will::ServerCliApp{}.parse(args.argc(), args.argv());

	CHECK(config.listen_port == 9000);
	CHECK(config.max_connections == 128);
	CHECK(config.db_path == "/tmp/custom.db");
	CHECK(config.keepalive_interval_seconds == 15);
	CHECK(config.keepalive_timeout_seconds == 5);
}
