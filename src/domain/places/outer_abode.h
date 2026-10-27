#pragma once

#include "matter/abode.h"
#include "matter/dweller.h"
#include "places/gates.h"
#include "places/place.h"
#include "properties/birth.h"

#include <functional>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>


namespace will::domain {


class Acquaintance;
class Contemplation;
class Man;
class Room;


/// An abode as it stands outward: its host, its dwellers, its gates, and the rooms set
/// outward, which neighbours see. Only the host brings one to dwell in it, each born anew
/// of his kind; the host's fathers by spirit dwell in it as ever friends, yet unseen among
/// the dwellers. Only its host acts in the places within it, and from within one goes out
/// only through its gates. Whether it shows a man a room, the whole abode says.
class OuterAbode : public Place {
public:
	~OuterAbode() override;

	const Man& host() const noexcept { return host_; }
	const Gates& gates() const noexcept { return gates_; }

	std::vector<std::reference_wrapper<const Room>> outer_rooms() const;

	void admit(Birth<Man> host, const matter::Dweller& kept);

	std::vector<std::shared_ptr<const Acquaintance>> dwellers() const;

	std::shared_ptr<const Acquaintance> dweller(const Man& man) const;

	bool dwells(const Man& man) const;

	virtual bool shows(const Man& man, const Room& room) const = 0;

	void host_only(const Contemplation& gaze, const Place& here) const;

	bool leads_to(const Man& who, const Place& next) const override;
	bool within(const OuterAbode& abode) const override;

protected:
	OuterAbode(Birth<Man> birth, const matter::Abode& kept);

	void withdraw_dwellers() const;

	mutable std::mutex rooms_mutex_;
	mutable std::vector<std::unique_ptr<Room>> outer_rooms_;

private:
	const Man& host_;
	Gates gates_;
	mutable std::mutex dwellers_mutex_;
	std::unordered_map<const Man*, std::shared_ptr<const Acquaintance>> dwellers_;
};


} // namespace will::domain
