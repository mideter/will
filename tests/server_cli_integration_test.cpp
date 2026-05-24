#include <cassert>
#include <cstdlib>
#include <fcntl.h>
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


RunResult run_will_server(const char* server_exe, const std::vector<std::string>& args)
{
    std::vector<std::string> argv_storage;
    argv_storage.reserve(args.size() + 1);
    argv_storage.emplace_back(server_exe);
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

        execv(server_exe, argv_ptrs.data());
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


void assert_help(const char* server_exe, const char* flag)
{
    const RunResult result = run_will_server(server_exe, {flag});
    assert(result.exit_code == 0);
    assert_contains(result.stderr_output, "Usage: will-server [options]");
    assert_contains(result.stderr_output, "--port PORT");
}


void assert_cli_error(const char* server_exe, const std::vector<std::string>& args,
                      int expected_exit_code, std::string_view error_fragment)
{
    const RunResult result = run_will_server(server_exe, args);
    assert(result.exit_code == expected_exit_code);
    assert_contains(result.stderr_output, error_fragment);
    assert_contains(result.stderr_output, "Usage: will-server [options]");
}


} // namespace


int main(int argc, char* argv[])
{
    if (argc < 2) {
        return EXIT_FAILURE;
    }

    const char* const server_exe = argv[1];

    assert_help(server_exe, "--help");
    assert_help(server_exe, "-h");

    assert_cli_error(server_exe, {"--unknown"}, 2, "Unknown option: --unknown");
    assert_cli_error(server_exe, {"--port"}, 2, "Invalid --port: requires a value");
    assert_cli_error(server_exe, {"--port", "0"}, 2, "Invalid --port:");
    assert_cli_error(server_exe, {"--io-threads", "abc"}, 2, "Invalid --io-threads: invalid value");
    assert_cli_error(server_exe, {"--help", "--port", "8080"}, 2,
                     "--help must be the only option");
    assert_cli_error(server_exe, {"--port", "8080", "--help"}, 2, "Unknown option: --help");

    return EXIT_SUCCESS;
}
