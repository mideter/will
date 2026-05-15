#include "sockethandle.h"

#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <system_error>

#include "socketerror.h"


namespace will {


SocketHandle::SocketHandle(int fd)
    : fd_(fd)
{
    if (fd_ < 0)
        throw std::system_error(errno, std::generic_category(), "socket failed");
}


SocketHandle::~SocketHandle()
{
    if (fd_ >= 0)
        ::close(fd_);
}


SocketHandle::SocketHandle(SocketHandle&& other) noexcept
    : fd_(other.fd_)
{
    other.fd_ = -1;
}


SocketHandle& SocketHandle::operator=(SocketHandle&& other) noexcept
{
    if (this != &other) {
        if (fd_ >= 0)
            ::close(fd_);

        fd_ = other.fd_;
        other.fd_ = -1;
    }

    return *this;
}


int SocketHandle::get() const
{
    return fd_;
}


void SocketHandle::enable_tcp_keepalive() const
{
    int on = 1;
    if (::setsockopt(fd_, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on)) < 0)
        throw SocketError("setsockopt SO_KEEPALIVE failed");

    const int keepidle = 60;
    const int keepintvl = 10;
    const int keepcnt = 3;

    if (::setsockopt(fd_, IPPROTO_TCP, TCP_KEEPIDLE, &keepidle, sizeof(keepidle)) < 0)
        throw SocketError("setsockopt TCP_KEEPIDLE failed");
    if (::setsockopt(fd_, IPPROTO_TCP, TCP_KEEPINTVL, &keepintvl, sizeof(keepintvl)) < 0)
        throw SocketError("setsockopt TCP_KEEPINTVL failed");
    if (::setsockopt(fd_, IPPROTO_TCP, TCP_KEEPCNT, &keepcnt, sizeof(keepcnt)) < 0)
        throw SocketError("setsockopt TCP_KEEPCNT failed");
}


} // namespace will
