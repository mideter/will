#pragma once

#include "serveraddress.h"


namespace will {


/** Фиксированный адрес сервера Will; клиенты не выбирают хост и порт. */
inline ServerAddress defaultWillServerAddress()
{
	return ServerAddress(IPv4("83.217.202.145"), Port(7770));
}


} // namespace will
