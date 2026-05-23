#include "serverclioptions.h"

#include "serverconfigparser.h"

#include <cstdlib>
#include <iostream>


namespace will {


bool PortCliOption::matches(std::string_view text) const
{
    return text == "--port";
}


std::string_view PortCliOption::primary_flag() const
{
    return "--port";
}


void PortCliOption::print_usage(std::ostream& os) const
{
    os << "  --port PORT                 Listen port " << ServerConfig::MinListenPort << '-'
       << ServerConfig::MaxListenPort << " (default " << ServerConfig::DefaultListenPort << ")\n";
}


void PortCliOption::apply(CliParserContext& context, ServerConfig& config) const
{
    config.set_listen_port(context.require_int(primary_flag()));
}


bool IoThreadsCliOption::matches(std::string_view text) const
{
    return text == "--io-threads";
}


std::string_view IoThreadsCliOption::primary_flag() const
{
    return "--io-threads";
}


void IoThreadsCliOption::print_usage(std::ostream& os) const
{
    os << "  --io-threads N              io_context worker threads (default "
       << ServerConfig::DefaultIoThreads << ")\n";
}


void IoThreadsCliOption::apply(CliParserContext& context, ServerConfig& config) const
{
    config.set_io_threads(context.require_int(primary_flag()));
}


bool ListenBacklogCliOption::matches(std::string_view text) const
{
    return text == "--listen-backlog";
}


std::string_view ListenBacklogCliOption::primary_flag() const
{
    return "--listen-backlog";
}


void ListenBacklogCliOption::print_usage(std::ostream& os) const
{
    os << "  --listen-backlog N          listen() backlog (default "
       << ServerConfig::DefaultListenBacklog << ")\n";
}


void ListenBacklogCliOption::apply(CliParserContext& context, ServerConfig& config) const
{
    config.set_listen_backlog(context.require_int(primary_flag()));
}


bool MaxClientsCliOption::matches(std::string_view text) const
{
    return text == "--max-clients";
}


std::string_view MaxClientsCliOption::primary_flag() const
{
    return "--max-clients";
}


void MaxClientsCliOption::print_usage(std::ostream& os) const
{
    os << "  --max-clients N             Max concurrent connections (default "
       << ServerConfig::DefaultMaxConnections << ")\n";
}


void MaxClientsCliOption::apply(CliParserContext& context, ServerConfig& config) const
{
    config.set_max_connections(context.require_size(primary_flag()));
}


bool MaxOutboundQueueCliOption::matches(std::string_view text) const
{
    return text == "--max-outbound-queue-bytes";
}


std::string_view MaxOutboundQueueCliOption::primary_flag() const
{
    return "--max-outbound-queue-bytes";
}


void MaxOutboundQueueCliOption::print_usage(std::ostream& os) const
{
    os << "  --max-outbound-queue-bytes N  Per-session write queue cap (default "
       << ServerConfig::DefaultMaxOutboundQueueBytes << ")\n";
}


void MaxOutboundQueueCliOption::apply(CliParserContext& context, ServerConfig& config) const
{
    config.set_max_outbound_queue_bytes(context.require_size(primary_flag()));
}


bool HelpCliOption::matches(std::string_view text) const
{
    return text == "--help" || text == "-h";
}


std::string_view HelpCliOption::primary_flag() const
{
    return "--help";
}


void HelpCliOption::print_usage(std::ostream& os) const
{
    (void)os;
}


void HelpCliOption::apply(CliParserContext& context, ServerConfig& config) const
{
    (void)context;
    (void)config;

    ServerConfigParser::print_usage();
    std::exit(0);
}


} // namespace will
