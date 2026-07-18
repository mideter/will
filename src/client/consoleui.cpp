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
constexpr char CheckMark[] = "✓";


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


void ConsoleUi::note_line_above_prompt_unlocked() const
{
    for (PendingMine& pending : pending_mines_)
        ++pending.rows_above_prompt;
}


void ConsoleUi::write_mine_line_unlocked(const std::string_view body,
                                         const bool dim,
                                         const bool acked) const
{
    if (color_ && dim) {
        std::cout << AnsiDim << "[me] " << body;
        if (acked)
            std::cout << ' ' << CheckMark;
        std::cout << AnsiReset;
        return;
    }

    if (color_)
        std::cout << AnsiGreen << "[me]" << AnsiReset << ' ' << body;
    else
        std::cout << "[me] " << body;

    if (acked) {
        if (color_)
            std::cout << ' ' << AnsiDim << CheckMark << AnsiReset;
        else
            std::cout << ' ' << CheckMark;
    }
}


void ConsoleUi::print_mine(const std::string_view body,
                           const bool dim,
                           const bool await_receipt) const
{
    std::lock_guard lock(console_mutex());

    write_mine_line_unlocked(body, dim, false);
    std::cout << std::endl;
    note_line_above_prompt_unlocked();

    if (!dim && await_receipt)
        pending_mines_.push_back(PendingMine{std::string{body}, 1});

    if (live_prompt_)
        print_prompt_unlocked();
}


void ConsoleUi::print_peer(const std::string_view name, const std::string_view body, const bool dim) const
{
    std::lock_guard lock(console_mutex());

    if (live_prompt_)
        std::cout << "\r\033[2K";

    const std::string_view tag = name.empty() ? "peer" : name;

    if (color_) {
        if (dim)
            std::cout << AnsiDim << '[' << tag << "] " << body << AnsiReset;
        else
            std::cout << AnsiYellow << '[' << tag << ']' << AnsiReset << ' ' << body;
    } else {
        std::cout << '[' << tag << "] " << body;
    }
    std::cout << std::endl;

    note_line_above_prompt_unlocked();

    if (live_prompt_)
        print_prompt_unlocked();
}


void ConsoleUi::print_receipt() const
{
    std::lock_guard lock(console_mutex());

    if (pending_mines_.empty())
        return;

    const PendingMine pending = std::move(pending_mines_.front());
    pending_mines_.pop_front();

    if (live_prompt_)
        std::cout << "\r\033[2K";

    if (pending.rows_above_prompt > 0)
        std::cout << "\033[" << pending.rows_above_prompt << 'A';

    std::cout << '\r' << "\033[2K";
    write_mine_line_unlocked(pending.body, false, true);

    if (pending.rows_above_prompt > 0)
        std::cout << "\033[" << pending.rows_above_prompt << 'B';

    std::cout << '\r';
    if (live_prompt_)
        print_prompt_unlocked();
    else
        std::cout.flush();
}


void ConsoleUi::print_status(const std::string_view text) const
{
    std::lock_guard lock(console_mutex());
    if (color_)
        std::cout << AnsiDim << text << AnsiReset << std::endl;
    else
        std::cout << text << std::endl;

    note_line_above_prompt_unlocked();
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
