#include "consoleui.h"

#include <algorithm>
#include <cstdlib>
#include <string>
#include <utility>
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
constexpr char AnsiYellow[] = "\033[33m";
constexpr char AnsiCyan[] = "\033[36m";
constexpr char CheckMark[] = "✓";


/// Columns a UTF-8 text takes: one to a character (the names here are narrow).
std::size_t width_of(const std::string_view text)
{
	std::size_t width = 0;
	for (const char c : text) {
		if ((static_cast<unsigned char>(c) & 0xC0) != 0x80)
			++width;
	}
	return width;
}


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
	, terminal_(isatty(STDOUT_FILENO) != 0)
{}


void ConsoleUi::print_prompt_unlocked() const
{
	if (color_)
		std::cout << AnsiGreen << AnsiBold << "> " << AnsiReset;
	else
		std::cout << "> ";
	std::cout.flush();
	prompt_shown_ = true;
}


bool ConsoleUi::clear_prompt_unlocked() const
{
	if (!live_prompt_ || !prompt_shown_)
		return false;

	prompt_shown_ = false;

	// In a terminal the prompt is wiped and drawn again below the line; elsewhere
	// the line follows it, and the prompt waits for the next input.
	if (!terminal_) {
		std::cout << '\n';
		return false;
	}
	std::cout << "\r\033[2K";
	return true;
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

	const bool prompted = clear_prompt_unlocked();

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

	if (prompted)
		print_prompt_unlocked();
}


void ConsoleUi::print_receipt() const
{
	std::lock_guard lock(console_mutex());

	// The line is redrawn with its mark only where the cursor can go back to it.
	if (pending_mines_.empty() || !terminal_)
		return;

	const PendingMine pending = std::move(pending_mines_.front());
	pending_mines_.pop_front();

	const bool prompted = clear_prompt_unlocked();

	if (pending.rows_above_prompt > 0)
		std::cout << "\033[" << pending.rows_above_prompt << 'A';

	std::cout << '\r' << "\033[2K";
	write_mine_line_unlocked(pending.body, false, true);

	if (pending.rows_above_prompt > 0)
		std::cout << "\033[" << pending.rows_above_prompt << 'B';

	std::cout << '\r';
	if (prompted)
		print_prompt_unlocked();
	else
		std::cout.flush();
}


void ConsoleUi::emit_line(const std::string_view line) const
{
	emit_lines({std::string{line}});
}


void ConsoleUi::emit_lines(const std::vector<std::string>& lines) const
{
	std::lock_guard lock(console_mutex());

	const bool prompted = clear_prompt_unlocked();
	for (const std::string& line : lines) {
		std::cout << line << '\n';
		note_line_above_prompt_unlocked();
	}
	std::cout.flush();
	if (prompted)
		print_prompt_unlocked();
}


std::string ConsoleUi::hinted(const std::string_view colour, const std::string_view text,
							  const std::string_view hint) const
{
	std::string line;
	if (color_ && !colour.empty())
		line += std::string{colour} + std::string{text} + AnsiReset;
	else
		line += text;
	if (!hint.empty()) {
		if (color_)
			line += AnsiDim;
		line += " — ";
		line += hint;
		if (color_)
			line += AnsiReset;
	}
	return line;
}


void ConsoleUi::print_status(const std::string_view text, const std::string_view hint) const
{
	emit_line(hinted({}, text, hint));
}


void ConsoleUi::print_header(const std::string_view text) const
{
	if (color_)
		emit_line(std::string{AnsiBold} + AnsiCyan + std::string{text} + AnsiReset);
	else
		emit_line(text);
}


void ConsoleUi::print_notice(const std::string_view text, const std::string_view hint) const
{
	emit_line(hinted(AnsiYellow, text, hint));
}


void ConsoleUi::print_list(const std::string_view title, const std::vector<std::vector<std::string>>& rows,
						   const std::string_view hint) const
{
	std::vector<std::string> lines{hinted({}, title, hint)};

	std::vector<std::size_t> widths;
	for (const std::vector<std::string>& row : rows) {
		if (widths.size() < row.size())
			widths.resize(row.size(), 0);
		for (std::size_t i = 0; i < row.size(); ++i)
			widths[i] = std::max(widths[i], width_of(row[i]));
	}

	for (const std::vector<std::string>& row : rows) {
		std::string line = "  ";
		for (std::size_t i = 0; i < row.size(); ++i) {
			line += row[i];
			if (i + 1 < row.size())
				line += std::string(widths[i] - width_of(row[i]) + 2, ' ');
		}
		lines.push_back(std::move(line));
	}
	emit_lines(lines);
}


void ConsoleUi::print_commands(const std::string_view group, const std::string_view commands) const
{
	std::string line;
	if (color_)
		line += AnsiBold;
	line += group;
	if (color_)
		line += AnsiReset;
	line += std::string(width_of(group) < 11 ? 11 - width_of(group) : 1, ' ');

	// Arguments, <…> and […], are dimmed.
	for (const char c : commands) {
		if (color_ && (c == '<' || c == '['))
			line += AnsiDim;
		line += c;
		if (color_ && (c == '>' || c == ']'))
			line += AnsiReset;
	}
	emit_line(line);
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
	emit_line(color_ ? std::string{AnsiDim} + "── history ──" + AnsiReset : std::string{"── history ──"});
}


void ConsoleUi::print_history_end() const
{
	emit_line(color_ ? std::string{AnsiDim} + "── live ──" + AnsiReset : std::string{"── live ──"});
}


void ConsoleUi::print_prompt() const
{
	std::lock_guard lock(console_mutex());
	if (!prompt_shown_)
		print_prompt_unlocked();
}


void ConsoleUi::set_live_prompt(const bool enabled) const
{
	std::lock_guard lock(console_mutex());
	if (!enabled)
		clear_prompt_unlocked();
	live_prompt_ = enabled;
}


void ConsoleUi::line_entered() const
{
	std::lock_guard lock(console_mutex());
	prompt_shown_ = false;
}


} // namespace will
