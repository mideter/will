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


constexpr std::string_view HelpCliOptionAliases[] = {"-h"};


const ServerCliOption ServerOptions[] = {
    {{"--port", CliValueType::Int, PrintPortUsage}, ApplyPort},
    {{"--io-threads", CliValueType::Int, PrintIoThreadsUsage}, ApplyIoThreads},
    {{"--listen-backlog", CliValueType::Int, PrintListenBacklogUsage}, ApplyListenBacklog},
    {{"--max-clients", CliValueType::Size, PrintMaxClientsUsage}, ApplyMaxClients},
    {{"--max-outbound-queue-bytes", CliValueType::Size, PrintMaxOutboundQueueUsage},
     ApplyMaxOutboundQueue},
};


} // namespace


const CliOption HelpCliOption{"--help", CliValueType::None, PrintHelpUsage, HelpCliOptionAliases};


std::span<const ServerCliOption> ServerCliOptions()
{
    return ServerOptions;
}


std::span<const CliOption> ServerCliOptionCliOptions()
{
    static const CliOption CliOptions[] = {
        ServerOptions[0].cli,
        ServerOptions[1].cli,
        ServerOptions[2].cli,
        ServerOptions[3].cli,
        ServerOptions[4].cli,
    };

    return CliOptions;
}


void ApplyServerCliOption(ServerConfig& config, const CliOptionMatch& match)
{
    for (const ServerCliOption& option : ServerCliOptions()) {
        if (option.cli.primary_flag() != match.primary_flag())
            continue;

        try {
            option.apply(config, match.value());
        } catch (const ServerConfigError& error) {
            throw CliInvalidOptionError(match.primary_flag(), error.what());
        }

        return;
    }
}


} // namespace will
