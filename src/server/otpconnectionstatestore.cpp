#include "otpconnectionstatestore.h"

#include "tcpconnectionregistry.h"

#include <chrono>
#include <utility>


namespace will {


OtpConnectionStateStore::OtpConnectionStateStore(TcpConnectionRegistry& registry,
                                               const int auth_pending_timeout_sec)
    : registry_(registry)
    , auth_pending_timeout_sec_(auth_pending_timeout_sec)
{}


OtpConnectionPhase OtpConnectionStateStore::phase(const std::uint64_t connection_id) const
{
    std::lock_guard lock(mutex_);
    const auto it = entries_.find(connection_id);

    if (it == entries_.end())
        return OtpConnectionPhase::None;

    return it->second.phase;
}


std::optional<std::string> OtpConnectionStateStore::pending_phone(const std::uint64_t connection_id) const
{
    std::lock_guard lock(mutex_);
    const auto it = entries_.find(connection_id);
    
    if (it == entries_.end() || it->second.phase != OtpConnectionPhase::AwaitingCode)
        return std::nullopt;

    return it->second.pending_phone;
}


void OtpConnectionStateStore::begin_awaiting_code(const std::uint64_t connection_id, std::string phone,
                                                  std::function<void(std::uint64_t)> on_timeout)
{
    std::uint64_t timer_generation = 0;

    {
        std::lock_guard lock(mutex_);
        Entry& entry = entries_[connection_id];
        cancel_timer(entry);
        entry.phase = OtpConnectionPhase::AwaitingCode;
        entry.pending_phone = std::move(phone);
        timer_generation = entry.timer_generation;
    }

    registry_.schedule_on_connection(connection_id,
                                     [this, connection_id, timer_generation,
                                      on_timeout = std::move(on_timeout)](asio::any_io_executor executor) {
                                         std::shared_ptr<asio::steady_timer> timer;

                                         {
                                             std::lock_guard lock(mutex_);
                                             const auto it = entries_.find(connection_id);
                                             if (it == entries_.end() || it->second.timer_generation != timer_generation)
                                                 return;

                                             timer = std::make_shared<asio::steady_timer>(executor);
                                             it->second.timer = timer;
                                         }

                                         timer->expires_after(std::chrono::seconds(auth_pending_timeout_sec_));
                                         timer->async_wait([this, connection_id, timer_generation, timer,
                                                            on_timeout = std::move(on_timeout)](
                                                               const asio::error_code& ec) {
                                             if (ec == asio::error::operation_aborted)
                                                 return;

                                             {
                                                 std::lock_guard lock(mutex_);
                                                 const auto it = entries_.find(connection_id);
                                                 if (it == entries_.end()
                                                     || it->second.timer_generation != timer_generation)
                                                     return;

                                                 it->second.phase = OtpConnectionPhase::None;
                                                 it->second.pending_phone.clear();
                                                 it->second.timer.reset();
                                             }

                                             on_timeout(connection_id);
                                         });
                                     });
}


void OtpConnectionStateStore::clear(const std::uint64_t connection_id)
{
    std::lock_guard lock(mutex_);
    const auto it = entries_.find(connection_id);
    if (it == entries_.end())
        return;

    cancel_timer(it->second);
    entries_.erase(it);
}


void OtpConnectionStateStore::cancel_timer(Entry& entry)
{
    if (entry.timer) {
        entry.timer->cancel();
        entry.timer.reset();
    }

    ++entry.timer_generation;
}


} // namespace will
