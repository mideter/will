#pragma once

#include "clioption.h"
#include "clioptioncursor.h"
#include "serverclioption.h"


namespace will {
namespace cli {


/** argv cursor with server option table matching built in. */
class ServerOptionCursor : public OptionCursor {
public:
    explicit ServerOptionCursor(int argc, char* argv[]);

    OptionMatch<ServerOption> match();
    void advance();
};


} // namespace cli
} // namespace will
