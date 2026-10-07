#pragma once

#include "clientconfig.h"

#include <deque>
#include <string>
#include <string_view>
#include <vector>


namespace will {


/** Terminal-aware chat printer: tagged lines, optional ANSI color, shared stdout lock. */
class ConsoleUi {
public:
	explicit ConsoleUi(ColorMode mode = ColorMode::Auto);

	bool color_enabled() const noexcept { return color_; }

	void print_mine(std::string_view body, bool dim = false, bool await_receipt = true) const;
	void print_peer(std::string_view name, std::string_view body, bool dim = false) const;
	void print_receipt() const;

	/// What is told: plain text and, after a dash, a dim hint of the command for it.
	void print_status(std::string_view text, std::string_view hint = {}) const;

	/// Where the gaze now rests.
	void print_header(std::string_view text) const;

	/// What the server answers or refuses, and what the console itself refuses;
	/// after a dash, a dim hint of the command for it.
	void print_notice(std::string_view text, std::string_view hint = {}) const;

	/// A list: its title (and hint), then a row to a line, indented, the cells
	/// of each column aligned.
	void print_list(std::string_view title, const std::vector<std::vector<std::string>>& rows,
					std::string_view hint = {}) const;

	/// A group of commands: its name, the commands, their arguments dimmed.
	void print_commands(std::string_view group, std::string_view commands) const;

	void print_error(std::string_view text) const;
	void print_history_begin() const;
	void print_history_end() const;
	/// Show the prompt, unless it is shown already.
	void print_prompt() const;

	/// A line was entered: the prompt it was typed after is no longer the last line.
	void line_entered() const;

	/** While true, inbound lines are followed by a reprinted prompt; turned off,
	 *  the prompt shown is wiped. */
	void set_live_prompt(bool enabled) const;

private:
	struct PendingMine {
		std::string body;
		int rows_above_prompt = 1;
	};

	static bool resolve_color(ColorMode mode);

	void print_prompt_unlocked() const;

	/// Clear the shown prompt before a line comes above it; whether it was shown.
	bool clear_prompt_unlocked() const;
	void write_mine_line_unlocked(std::string_view body, bool dim, bool acked) const;
	void note_line_above_prompt_unlocked() const;

	/// Put lines above the prompt, together: the prompt is cleared before them and
	/// drawn again after.
	void emit_line(std::string_view line) const;
	void emit_lines(const std::vector<std::string>& lines) const;

	/// A text in this colour, and the dim hint after a dash.
	std::string hinted(std::string_view colour, std::string_view text, std::string_view hint) const;

	bool color_;
	/// The output is a terminal: the prompt may be cleared and drawn again.
	bool terminal_;
	mutable bool live_prompt_ = false;
	mutable bool prompt_shown_ = false;
	mutable std::deque<PendingMine> pending_mines_;
};


} // namespace will
