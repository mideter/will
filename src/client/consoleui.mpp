module;

#include <deque>
#include <string>
#include <string_view>

export module will.client.consoleui;

export import will.client.clientconfig;

export namespace will {


/** Terminal-aware chat printer: tagged lines, optional ANSI color, shared stdout lock. */
class ConsoleUi {
public:
    explicit ConsoleUi(ColorMode mode = ColorMode::Auto);

    bool color_enabled() const noexcept { return color_; }

    void print_mine(std::string_view body, bool dim = false, bool await_receipt = true) const;
    void print_peer(std::string_view name, std::string_view body, bool dim = false) const;
    void print_receipt() const;
    void print_status(std::string_view text) const;
    void print_error(std::string_view text) const;
    void print_history_begin() const;
    void print_history_end() const;
    void print_prompt() const;

    /** While true, inbound lines are followed by a reprinted prompt. */
    void set_live_prompt(bool enabled) noexcept { live_prompt_ = enabled; }

private:
    struct PendingMine {
        std::string body;
        int rows_above_prompt = 1;
    };

    static bool resolve_color(ColorMode mode);

    void print_prompt_unlocked() const;
    void write_mine_line_unlocked(std::string_view body, bool dim, bool acked) const;
    void note_line_above_prompt_unlocked() const;

    bool color_;
    bool live_prompt_ = false;
    mutable std::deque<PendingMine> pending_mines_;
};


} // namespace will
