#include "serverclioptions.h"

#include "serverconfig.h"

#include <iostream>


namespace will {


PortCliOption::PortCliOption()
    : CliOption("--port", CliValueType::Int)
{}


const PortCliOption& PortCliOption::instance()
{
    static const PortCliOption option;
    return option;
}


void PortCliOption::print_usage(std::ostream& os) const
{
    os << "  --port PORT                 Listen port " << ServerConfig::MinListenPort << '-'
       << ServerConfig::MaxListenPort << " (default " << ServerConfig::DefaultListenPort << ")\n";
}


IoThreadsCliOption::IoThreadsCliOption()
    : CliOption("--io-threads", CliValueType::Int)
{}


const IoThreadsCliOption& IoThreadsCliOption::instance()
{
    static const IoThreadsCliOption option;
    return option;
}


void IoThreadsCliOption::print_usage(std::ostream& os) const
{
    os << "  --io-threads N              io_context worker threads (default "
       << ServerConfig::DefaultIoThreads << ")\n";
}


ListenBacklogCliOption::ListenBacklogCliOption()
    : CliOption("--listen-backlog", CliValueType::Int)
{}


const ListenBacklogCliOption& ListenBacklogCliOption::instance()
{
    static const ListenBacklogCliOption option;
    return option;
}


void ListenBacklogCliOption::print_usage(std::ostream& os) const
{
    os << "  --listen-backlog N          listen() backlog (default "
       << ServerConfig::DefaultListenBacklog << ")\n";
}


MaxClientsCliOption::MaxClientsCliOption()
    : CliOption("--max-clients", CliValueType::Size)
{}


const MaxClientsCliOption& MaxClientsCliOption::instance()
{
    static const MaxClientsCliOption option;
    return option;
}


void MaxClientsCliOption::print_usage(std::ostream& os) const
{
    os << "  --max-clients N             Max concurrent connections (default "
       << ServerConfig::DefaultMaxConnections << ")\n";
}


MaxOutboundQueueCliOption::MaxOutboundQueueCliOption()
    : CliOption("--max-outbound-queue-bytes", CliValueType::Size)
{}


const MaxOutboundQueueCliOption& MaxOutboundQueueCliOption::instance()
{
    static const MaxOutboundQueueCliOption option;
    return option;
}


void MaxOutboundQueueCliOption::print_usage(std::ostream& os) const
{
    os << "  --max-outbound-queue-bytes N  Per-session write queue cap (default "
       << ServerConfig::DefaultMaxOutboundQueueBytes << ")\n";
}


HelpCliOption::HelpCliOption()
    : CliOption("--help", CliValueType::None)
{}


const HelpCliOption& HelpCliOption::instance()
{
    static const HelpCliOption option;
    return option;
}


bool HelpCliOption::matches(std::string_view text) const
{
    return text == flag() || text == "-h";
}


void HelpCliOption::print_usage(std::ostream& os) const
{
    (void)os;
}


const std::array<const CliOption*, 6>& all_server_cli_options()
{
    static const std::array<const CliOption*, 6> options{
        &PortCliOption::instance(),
        &IoThreadsCliOption::instance(),
        &ListenBacklogCliOption::instance(),
        &MaxClientsCliOption::instance(),
        &MaxOutboundQueueCliOption::instance(),
        &HelpCliOption::instance(),
    };
    return options;
}


const std::array<const CliOption*, 5>& config_server_cli_options()
{
    static const std::array<const CliOption*, 5> options{
        &PortCliOption::instance(),
        &IoThreadsCliOption::instance(),
        &ListenBacklogCliOption::instance(),
        &MaxClientsCliOption::instance(),
        &MaxOutboundQueueCliOption::instance(),
    };
    return options;
}


} // namespace will
