#include "serverclioptioncursor.h"


namespace will {
namespace cli {


ServerOptionCursor::ServerOptionCursor(int argc, char* argv[])
    : OptionCursor(argc, argv)
{}


OptionMatch<ServerOption> ServerOptionCursor::match()
{
    return OptionMatch<ServerOption>{*this, ServerOptionTable::ServerOptions};
}


void ServerOptionCursor::advance()
{
    (*this)++;
}


} // namespace cli
} // namespace will
