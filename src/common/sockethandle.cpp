#include "sockethandle.h"

#include <unistd.h>

#include <cerrno>
#include <system_error>


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


} // namespace will
