#include "serverclioption.h"

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


void ApplyPort(ServerConfig& config, const ServerCliOption<IntValue>::Value& value)
{
    config.set_listen_port(std::get<int>(value));
}


void ApplyIoThreads(ServerConfig& config, const ServerCliOption<IntValue>::Value& value)
{
    config.set_io_threads(std::get<int>(value));
}


void ApplyListenBacklog(ServerConfig& config, const ServerCliOption<IntValue>::Value& value)
{
    config.set_listen_backlog(std::get<int>(value));
}


void ApplyMaxClients(ServerConfig& config, const ServerCliOption<SizeValue>::Value& value)
{
    config.set_max_connections(std::get<std::size_t>(value));
}


void ApplyMaxOutboundQueue(ServerConfig& config, const ServerCliOption<SizeValue>::Value& value)
{
    config.set_max_outbound_queue_bytes(std::get<std::size_t>(value));
}


} // namespace


const CliOption<NoneValue> ServerCliOptionTable::HelpOption{
    "--help", PrintHelpUsage, HelpOptionAliases};


const std::array<ServerOption, 5> ServerCliOptionTable::ServerOptions = {
    ServerCliOption<IntValue>{ApplyPort, "--port", PrintPortUsage},
    ServerCliOption<IntValue>{ApplyIoThreads, "--io-threads", PrintIoThreadsUsage},
    ServerCliOption<IntValue>{ApplyListenBacklog, "--listen-backlog", PrintListenBacklogUsage},
    ServerCliOption<SizeValue>{ApplyMaxClients, "--max-clients", PrintMaxClientsUsage},
    ServerCliOption<SizeValue>{ApplyMaxOutboundQueue, "--max-outbound-queue-bytes",
                             PrintMaxOutboundQueueUsage},
};


template<typename ValueTag>
ServerCliOption<ValueTag>::ServerCliOption(Applier applier, std::string_view flag,
                                           CliUsagePrinter print_usage,
                                           std::span<const std::string_view> aliases)
    : CliOption<ValueTag>(flag, std::move(print_usage), aliases)
    , applier_(std::move(applier))
{}


template class ServerCliOption<IntValue>;
template class ServerCliOption<SizeValue>;

} // namespace will
