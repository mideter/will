#pragma once

#include "clioption.h"
#include "clioptioncursor.h"
#include "serverclioption.h"


namespace will {


/** argv cursor with server option table matching built in. */
class ServerCliOptionCursor : public CliOptionCursor {
public:
    explicit ServerCliOptionCursor(int argc, char* argv[]);

    CliOptionMatch<ServerOption> match();
    void advance();
};


} // namespace will
