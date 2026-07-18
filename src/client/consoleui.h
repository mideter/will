#pragma once

#include "clientconfig.h"

#include <string_view>


namespace will {


/** Terminal-aware chat printer: tagged lines, optional ANSI color, shared stdout lock. */
class ConsoleUi {
public:
    explicit ConsoleUi(ColorMode mode = ColorMode::Auto);

    bool color_enabled() const noexcept { return color_; }

    void print_mine(std::string_view body, bool dim = false) const;
    void print_peer(std::string_view body, bool dim = false) const;
    void print_server(std::string_view body) const;
    void print_status(std::string_view text) const;
    void print_error(std::string_view text) const;
    void print_history_begin() const;
    void print_history_end() const;
    void print_prompt() const;

    /** While true, inbound lines are followed by a reprinted prompt. */
    void set_live_prompt(bool enabled) noexcept { live_prompt_ = enabled; }

private:
    static bool resolve_color(ColorMode mode);

    void print_chat_line(std::string_view tag,
                         std::string_view tag_sgr,
                         std::string_view body,
                         bool dim,
                         bool interrupt_prompt) const;
    void print_prompt_unlocked() const;

    bool color_;
    bool live_prompt_ = false;
};


} // namespace will
