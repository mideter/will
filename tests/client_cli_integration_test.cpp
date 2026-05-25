#include <CLI/CLI.hpp>

#include <cassert>
#include <cstdlib>
#include <fcntl.h>
#include <iostream>
#include <string>
#include <string_view>
#include <sys/wait.h>
#include <unistd.h>
#include <vector>


namespace {


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


void assert_contains(std::string_view haystack, std::string_view needle)
{
    assert(haystack.find(needle) != std::string_view::npos);
}


void run_case(std::string_view name, const auto& test_fn)
{
    std::cout << "  " << name << " ... ";
    test_fn();
    std::cout << "ok\n";
}


void check_help(const char* client_exe, const std::vector<std::string>& args)
{
    const RunResult result = run_will_client(client_exe, args);
    assert(result.exit_code == 0);
    assert_contains(result.stderr_output, "will-client");
    assert_contains(result.stderr_output, "--host");
    assert_contains(result.stderr_output, "--help");
}


void check_cli_error(const char* client_exe, const std::vector<std::string>& args,
                     int expected_exit_code, std::string_view error_fragment)
{
    const RunResult result = run_will_client(client_exe, args);
    assert(result.exit_code == expected_exit_code);
    assert_contains(result.stderr_output, error_fragment);
    assert_contains(result.stderr_output, "help");
}


void print_usage(const char* program)
{
    std::cerr << "Usage: " << program << " <path-to-will-client>\n"
              << "\n"
              << "Integration tests for ClientConfigParser / will-client CLI behavior.\n"
              << "Example:\n"
              << "  " << program << " ./build/will-client\n"
              << "\n"
              << "Or via CTest (path is passed automatically):\n"
              << "  ctest --test-dir build -R will-client-cli-integration --output-on-failure\n";
}


} // namespace


int main(int argc, char* argv[])
{
    if (argc < 2) {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    const char* const client_exe = argv[1];

    std::cout << "will-client CLI integration tests\n"
              << "  binary: " << client_exe << '\n'
              << '\n';

    run_case("--help prints usage and exits 0", [&] { check_help(client_exe, {"--help"}); });

    run_case("-h prints usage and exits 0", [&] { check_help(client_exe, {"-h"}); });

    run_case("--help with other options still prints help and exits 0", [&] {
        check_help(client_exe, {"--help", "--port", "8080"});
    });

    run_case("--help after other options prints help and exits 0", [&] {
        check_help(client_exe, {"--port", "8080", "--help"});
    });

    run_case("--unknown prints error and usage, exits ExtrasError", [&] {
        check_cli_error(client_exe, {"--unknown"},
                        static_cast<int>(CLI::ExitCodes::ExtrasError), "--unknown");
    });

    run_case("--port without value prints error and usage, exits ArgumentMismatch", [&] {
        check_cli_error(client_exe, {"--port"},
                        static_cast<int>(CLI::ExitCodes::ArgumentMismatch), "--port");
    });

    run_case("--port 0 fails at client startup, exits 1", [&] {
        const RunResult result = run_will_client(client_exe, {"--port", "0"});
        assert(result.exit_code == 1);
        assert_contains(result.stderr_output, "Client error:");
        assert_contains(result.stderr_output, "port");
    });

    run_case("--host invalid fails at client startup, exits 1", [&] {
        const RunResult result = run_will_client(client_exe, {"--host", "invalid"});
        assert(result.exit_code == 1);
        assert_contains(result.stderr_output, "Client error:");
        assert_contains(result.stderr_output, "host");
    });

    run_case("--port abc prints error and usage, exits ConversionError", [&] {
        check_cli_error(client_exe, {"--port", "abc"},
                        static_cast<int>(CLI::ExitCodes::ConversionError), "--port");
    });

    std::cout << "\nAll tests passed.\n";
    return EXIT_SUCCESS;
}
