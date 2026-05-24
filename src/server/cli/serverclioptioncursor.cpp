#include "serverclioptioncursor.h"


namespace will {
namespace cli {


ServerCliOptionCursor::ServerCliOptionCursor(int argc, char* argv[])
    : CliOptionCursor(argc, argv)
{}


CliOptionMatch<ServerOption> ServerCliOptionCursor::match()
{
    return CliOptionMatch<ServerOption>{*this, ServerCliOptionTable::ServerOptions};
}


void ServerCliOptionCursor::advance()
{
    (*this)++;
}


} // namespace cli
} // namespace will
