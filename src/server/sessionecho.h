#pragma once

#include "ports/echo.h"
#include "entities/heaven.h"


namespace will {


class SessionRegistry;


class SessionEcho final : public domain::Echo {
public:
	SessionEcho(SessionRegistry& registry, domain::Heaven& heaven);

	void notify_letter(const domain::Letter& letter) override;

private:
	SessionRegistry& registry_;
	domain::Heaven& heaven_;
};


} // namespace will
