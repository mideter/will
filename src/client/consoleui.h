#pragma once

#include "clientconfig.h"

#include <deque>
#include <string>
#include <string_view>
#include <vector>


namespace will {


/// Terminal-aware chat printer: tagged lines, optional ANSI color, shared stdout lock. A hint
/// of the command follows a dash, dimmed. Lines that come go above the prompt, together: it
/// is cleared before them and, while live, drawn again after; turned off, it is wiped. A list
/// is its title, then a row to a line with the cells of each column aligned.
class ConsoleUi {
public:
	explicit ConsoleUi(ColorMode mode = ColorMode::Auto);

	void print_mine(std::string_view body, bool dim = false, bool await_receipt = true) const;
	void print_peer(std::string_view name, std::string_view body, bool dim = false) const;
	void print_receipt() const;

	void print_status(std::string_view text, std::string_view hint = {}) const;

	void print_header(std::string_view text) const;

	void print_notice(std::string_view text, std::string_view hint = {}) const;

	void print_list(std::string_view title, const std::vector<std::vector<std::string>>& rows,
					std::string_view hint = {}) const;

	void print_commands(std::string_view group, std::string_view commands) const;

	void print_error(std::string_view text) const;
	void print_history_begin() const;
	void print_history_end() const;
	void print_prompt() const;

	void line_entered() const;

	void set_live_prompt(bool enabled) const;

private:
	struct PendingMine {
		std::string body;
		int rows_above_prompt = 1;
	};

	static bool resolve_color(ColorMode mode);

	void print_prompt_unlocked() const;

	bool clear_prompt_unlocked() const;
	void write_mine_line_unlocked(std::string_view body, bool dim, bool acked) const;
	void note_line_above_prompt_unlocked() const;

	void emit_line(std::string_view line) const;
	void emit_lines(const std::vector<std::string>& lines) const;

	std::string hinted(std::string_view colour, std::string_view text, std::string_view hint) const;

	bool color_;
	bool terminal_;
	mutable bool live_prompt_ = false;
	mutable bool prompt_shown_ = false;
	mutable std::deque<PendingMine> pending_mines_;
};


} // namespace will
