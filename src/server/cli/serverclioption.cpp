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


void ApplyPort(ServerConfig& config, const CliOptionMatch::Value& value)
{
    config.set_listen_port(std::get<int>(value));
}


void ApplyIoThreads(ServerConfig& config, const CliOptionMatch::Value& value)
{
    config.set_io_threads(std::get<int>(value));
}


void ApplyListenBacklog(ServerConfig& config, const CliOptionMatch::Value& value)
{
    config.set_listen_backlog(std::get<int>(value));
}


void ApplyMaxClients(ServerConfig& config, const CliOptionMatch::Value& value)
{
    config.set_max_connections(std::get<std::size_t>(value));
}


void ApplyMaxOutboundQueue(ServerConfig& config, const CliOptionMatch::Value& value)
{
    config.set_max_outbound_queue_bytes(std::get<std::size_t>(value));
}


} // namespace


const CliOption ServerCliOption::HelpOption{
    "--help", CliValueType::None, PrintHelpUsage, HelpOptionAliases};


const std::array<ServerCliOption, 5> ServerCliOption::ServerOptions = {
    ServerCliOption{ApplyPort, "--port", CliValueType::Int, PrintPortUsage},
    ServerCliOption{ApplyIoThreads, "--io-threads", CliValueType::Int, PrintIoThreadsUsage},
    ServerCliOption{ApplyListenBacklog, "--listen-backlog", CliValueType::Int, PrintListenBacklogUsage},
    ServerCliOption{ApplyMaxClients, "--max-clients", CliValueType::Size, PrintMaxClientsUsage},
    ServerCliOption{ApplyMaxOutboundQueue, "--max-outbound-queue-bytes", CliValueType::Size,
                    PrintMaxOutboundQueueUsage},
};


ServerCliOption::ServerCliOption(Applier applier, const std::string_view flag,
                                 const CliValueType value_type, UsagePrinter print_usage,
                                 const std::span<const std::string_view> aliases)
    : CliOption(flag, value_type, std::move(print_usage), aliases)
    , applier_(std::move(applier))
{}


void ServerCliOption::apply(ServerConfig& config, const CliOptionMatch::Value& value) const
{
    applier_(config, value);
}


} // namespace will
