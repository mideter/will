#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <CLI/CLI.hpp>

#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>


namespace {


const char* g_client_exe = nullptr;


struct RunResult {
	int exit_code = -1;
	std::string stderr_output;
};


RunResult run_will_client(const char* client_exe, const std::vector<std::string>& args)
{
	std::vector<std::string> argv_storage;
	argv_storage.reserve(args.size() + 1);
	argv_storage.emplace_back(client_exe);
	argv_storage.insert(argv_storage.end(), args.begin(), args.end());

	std::vector<char*> argv_ptrs;
	argv_ptrs.reserve(argv_storage.size() + 1);
	for (std::string& arg : argv_storage)
		argv_ptrs.push_back(arg.data());
	argv_ptrs.push_back(nullptr);

	int pipefd[2] = {-1, -1};
	if (pipe(pipefd) != 0)
		return {};

	const pid_t pid = fork();
	if (pid < 0) {
		close(pipefd[0]);
		close(pipefd[1]);
		return {};
	}

	if (pid == 0) {
		close(pipefd[0]);

		const int nullfd = open("/dev/null", O_WRONLY);
		if (nullfd >= 0) {
			dup2(nullfd, STDOUT_FILENO);
			close(nullfd);
		}

		dup2(pipefd[1], STDERR_FILENO);
		close(pipefd[1]);

		execv(client_exe, argv_ptrs.data());
		_exit(127);
	}

	close(pipefd[1]);

	RunResult result;
	char buffer[512];
	for (;;) {
		const ssize_t bytes_read = read(pipefd[0], buffer, sizeof buffer);
		if (bytes_read <= 0)
			break;
		result.stderr_output.append(buffer, static_cast<std::size_t>(bytes_read));
	}
	close(pipefd[0]);

	int status = 0;
	if (waitpid(pid, &status, 0) < 0)
		return result;

	if (WIFEXITED(status))
		result.exit_code = WEXITSTATUS(status);

	return result;
}


void check_contains(std::string_view haystack, std::string_view needle)
{
	CHECK(haystack.find(needle) != std::string_view::npos);
}


void check_help(const std::vector<std::string>& args)
{
	const RunResult result = run_will_client(g_client_exe, args);
	CHECK(result.exit_code == 0);
	check_contains(result.stderr_output, "will-client");
	check_contains(result.stderr_output, "--host");
	check_contains(result.stderr_output, "--help");
}


void check_cli_error(const std::vector<std::string>& args, int expected_exit_code,
					 std::string_view error_fragment)
{
	const RunResult result = run_will_client(g_client_exe, args);
	CHECK(result.exit_code == expected_exit_code);
	check_contains(result.stderr_output, error_fragment);
	check_contains(result.stderr_output, "help");
}


} // namespace


TEST_CASE("will-client --help prints usage and exits 0")
{
	check_help({"--help"});
}


TEST_CASE("will-client -h prints usage and exits 0")
{
	check_help({"-h"});
}


TEST_CASE("will-client --help with other options")
{
	check_help({"--help", "--port", "8080"});
}


TEST_CASE("will-client --help after other options")
{
	check_help({"--port", "8080", "--help"});
}


TEST_CASE("will-client --unknown exits ExtrasError")
{
	check_cli_error({"--unknown"}, static_cast<int>(CLI::ExitCodes::ExtrasError), "--unknown");
}


TEST_CASE("will-client --port without value exits ArgumentMismatch")
{
	check_cli_error({"--port"}, static_cast<int>(CLI::ExitCodes::ArgumentMismatch), "--port");
}


TEST_CASE("will-client --port 0 fails at startup")
{
	const RunResult result = run_will_client(g_client_exe, {"--port", "0"});
	CHECK(result.exit_code == 1);
	check_contains(result.stderr_output, "Client error:");
	check_contains(result.stderr_output, "port");
}


TEST_CASE("will-client --host invalid fails at startup")
{
	const RunResult result = run_will_client(g_client_exe, {"--host", "invalid"});
	CHECK(result.exit_code == 1);
	check_contains(result.stderr_output, "Client error:");
	check_contains(result.stderr_output, "host");
}


TEST_CASE("will-client --port abc exits ConversionError")
{
	check_cli_error({"--port", "abc"}, static_cast<int>(CLI::ExitCodes::ConversionError), "--port");
}


int main(int argc, char** argv)
{
	if (argc < 2) {
		std::cerr << "Usage: " << argv[0] << " <path-to-will-client>\n";
		return EXIT_FAILURE;
	}

	g_client_exe = argv[1];

	doctest::Context context;
	std::vector<char*> doctest_argv{argv[0]};
	for (int i = 2; i < argc; ++i)
		doctest_argv.push_back(argv[i]);
	context.applyCommandLine(static_cast<int>(doctest_argv.size()), doctest_argv.data());
	return context.run();
}
