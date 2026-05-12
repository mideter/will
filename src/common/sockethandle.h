#pragma once


namespace will {


class SocketHandle {
public:
	explicit SocketHandle(int fd);
	~SocketHandle();

	SocketHandle(const SocketHandle&) = delete;
	SocketHandle& operator=(const SocketHandle&) = delete;

	SocketHandle(SocketHandle&& other) noexcept;
	SocketHandle& operator=(SocketHandle&& other) noexcept;

	int get() const;

	/** TCP keepalive: SO_KEEPALIVE and TCP_KEEPIDLE / TCP_KEEPINTVL / TCP_KEEPCNT. */
	void enable_tcp_keepalive() const;

private:
	int fd_;
};


} // namespace will
