#include "serverclioptions.h"

#include "clioption.h"
#include "serverconfig.h"

#include <iostream>


namespace will {


namespace {


void PrintPortUsage(std::ostream& os)
{
    os << "  --port PORT                 Listen port " << ServerConfig::MinListenPort << '-'
       << ServerConfig::MaxListenPort << " (default " << ServerConfig::DefaultListenPort << ")\n";
}


void PrintIoThreadsUsage(std::ostream& os)
{
    os << "  --io-threads N              io_context worker threads (default "
       << ServerConfig::DefaultIoThreads << ")\n";
}


void PrintListenBacklogUsage(std::ostream& os)
{
    os << "  --listen-backlog N          listen() backlog (default "
       << ServerConfig::DefaultListenBacklog << ")\n";
}


void PrintMaxClientsUsage(std::ostream& os)
{
    os << "  --max-clients N             Max concurrent connections (default "
       << ServerConfig::DefaultMaxConnections << ")\n";
}


void PrintMaxOutboundQueueUsage(std::ostream& os)
{
    os << "  --max-outbound-queue-bytes N  Per-session write queue cap (default "
       << ServerConfig::DefaultMaxOutboundQueueBytes << ")\n";
}


void PrintHelpUsage(std::ostream& os)
{
    os << "  --help, -h                  Show this help\n";
}


constexpr std::string_view HelpOptionAliases[] = {"-h"};


} // namespace


std::span<const CliOption> AllServerCliOptions()
{
    static const CliOption Options[] = {
        {"--port", CliValueType::Int, PrintPortUsage},
        {"--io-threads", CliValueType::Int, PrintIoThreadsUsage},
        {"--listen-backlog", CliValueType::Int, PrintListenBacklogUsage},
        {"--max-clients", CliValueType::Size, PrintMaxClientsUsage},
        {"--max-outbound-queue-bytes", CliValueType::Size, PrintMaxOutboundQueueUsage},
        {"--help", CliValueType::None, PrintHelpUsage, HelpOptionAliases},
    };

    return Options;
}


} // namespace will
