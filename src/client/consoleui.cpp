#include "consoleui.h"

#include <cstdlib>
#include <iostream>
#include <mutex>
#include <unistd.h>


namespace will {


namespace {


constexpr char AnsiReset[] = "\033[0m";
constexpr char AnsiDim[] = "\033[2m";
constexpr char AnsiBold[] = "\033[1m";
constexpr char AnsiRed[] = "\033[31m";
constexpr char AnsiGreen[] = "\033[32m";
constexpr char AnsiCyan[] = "\033[36m";
constexpr char AnsiYellow[] = "\033[33m";
constexpr char AnsiMagenta[] = "\033[35m";


std::mutex& console_mutex()
{
    static std::mutex mutex;
    return mutex;
}


} // namespace


bool ConsoleUi::resolve_color(const ColorMode mode)
{
    if (mode == ColorMode::Always)
        return true;
    if (mode == ColorMode::Never)
        return false;
    if (std::getenv("NO_COLOR") != nullptr)
        return false;
    return isatty(STDOUT_FILENO) != 0;
}


ConsoleUi::ConsoleUi(const ColorMode mode)
    : color_(resolve_color(mode))
{}


void ConsoleUi::print_prompt_unlocked() const
{
    if (color_)
        std::cout << AnsiCyan << AnsiBold << "> " << AnsiReset;
    else
        std::cout << "> ";
    std::cout.flush();
}


void ConsoleUi::print_chat_line(const std::string_view tag,
                                const std::string_view tag_sgr,
                                const std::string_view body,
                                const bool dim,
                                const bool interrupt_prompt) const
{
    std::lock_guard lock(console_mutex());

    if (interrupt_prompt)
        std::cout << "\r\033[2K";

    if (color_) {
        if (dim)
            std::cout << AnsiDim << tag << ' ' << body << AnsiReset;
        else
            std::cout << tag_sgr << tag << AnsiReset << ' ' << body;
    } else {
        std::cout << tag << ' ' << body;
    }
    std::cout << std::endl;

    if (interrupt_prompt)
        print_prompt_unlocked();
}


void ConsoleUi::print_mine(const std::string_view body, const bool dim) const
{
    print_chat_line("[me]", AnsiGreen, body, dim, false);
}


void ConsoleUi::print_peer(const std::string_view body, const bool dim) const
{
    print_chat_line("[peer]", AnsiYellow, body, dim, live_prompt_);
}


void ConsoleUi::print_server(const std::string_view body) const
{
    std::lock_guard lock(console_mutex());
    if (color_)
        std::cerr << AnsiMagenta << "[server]" << AnsiReset << ' ' << body << std::endl;
    else
        std::cerr << "[server] " << body << std::endl;
}


void ConsoleUi::print_status(const std::string_view text) const
{
    std::lock_guard lock(console_mutex());
    if (color_)
        std::cout << AnsiDim << text << AnsiReset << std::endl;
    else
        std::cout << text << std::endl;
}


void ConsoleUi::print_error(const std::string_view text) const
{
    std::lock_guard lock(console_mutex());
    if (color_)
        std::cerr << AnsiRed << text << AnsiReset << '\n';
    else
        std::cerr << text << '\n';
}


void ConsoleUi::print_history_begin() const
{
    print_status("── history ──");
}


void ConsoleUi::print_history_end() const
{
    print_status("── live ──");
}


void ConsoleUi::print_prompt() const
{
    std::lock_guard lock(console_mutex());
    print_prompt_unlocked();
}


} // namespace will
