#include "serverclioption.h"

#include <iostream>


namespace will {
namespace cli {


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


void ApplyPort(ServerConfig& config, const ParsedValue& value)
{
    config.set_listen_port(std::get<int>(value));
}


void ApplyIoThreads(ServerConfig& config, const ParsedValue& value)
{
    config.set_io_threads(std::get<int>(value));
}


void ApplyListenBacklog(ServerConfig& config, const ParsedValue& value)
{
    config.set_listen_backlog(std::get<int>(value));
}


void ApplyMaxClients(ServerConfig& config, const ParsedValue& value)
{
    config.set_max_connections(std::get<std::size_t>(value));
}


void ApplyMaxOutboundQueue(ServerConfig& config, const ParsedValue& value)
{
    config.set_max_outbound_queue_bytes(std::get<std::size_t>(value));
}


} // namespace


const Option<NoneValue> ServerOptionTable::HelpOption{
    "--help", PrintHelpUsage, HelpOptionAliases};


const std::array<ServerOption, 5> ServerOptionTable::ServerOptions = {
    ConfigOption<IntValue>{ApplyPort, "--port", PrintPortUsage},
    ConfigOption<IntValue>{ApplyIoThreads, "--io-threads", PrintIoThreadsUsage},
    ConfigOption<IntValue>{ApplyListenBacklog, "--listen-backlog", PrintListenBacklogUsage},
    ConfigOption<SizeValue>{ApplyMaxClients, "--max-clients", PrintMaxClientsUsage},
    ConfigOption<SizeValue>{ApplyMaxOutboundQueue, "--max-outbound-queue-bytes",
                             PrintMaxOutboundQueueUsage},
};


template<typename ValueTag>
    requires(std::derived_from<ValueTag, Value> && !std::is_same_v<ValueTag, NoneValue>)
ConfigOption<ValueTag>::ConfigOption(Applier applier, std::string_view flag,
                                           UsagePrinter print_usage,
                                           std::span<const std::string_view> aliases)
    : Option<ValueTag>(flag, std::move(print_usage), aliases)
    , applier_(std::move(applier))
{}


template<typename ValueTag>
    requires(std::derived_from<ValueTag, Value> && !std::is_same_v<ValueTag, NoneValue>)
void ConfigOption<ValueTag>::apply(ServerConfig& config, const ParsedValue& value) const
{
    applier_(config, value);
}


template class ConfigOption<IntValue>;
template class ConfigOption<SizeValue>;

} // namespace cli
} // namespace will
