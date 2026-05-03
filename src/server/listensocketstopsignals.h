#pragma once

#include <csignal>


class ListenSocketStopSignals {
public:
	explicit ListenSocketStopSignals(int listen_fd);
	~ListenSocketStopSignals();

	ListenSocketStopSignals(const ListenSocketStopSignals&) = delete;
	ListenSocketStopSignals& operator=(const ListenSocketStopSignals&) = delete;

	bool shutdown_requested() const noexcept;

private:
	struct sigaction old_term_ {};
	struct sigaction old_int_ {};
};
