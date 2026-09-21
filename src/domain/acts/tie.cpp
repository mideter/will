#include "tie.h"

#include "beings/novice.h"
#include "beings/soul.h"
#include "beings/testator.h"

#include <stdexcept>
#include <unordered_map>
#include <utility>


namespace will::domain {
namespace {


std::unordered_map<std::uint64_t, const Tie*>& registry()
{
	static std::unordered_map<std::uint64_t, const Tie*> ties;
	return ties;
}


} // namespace


Tie::Tie(Tying tying)
	: Place(id::Place{tying.id().value()})
	, Obedience(tying.id())
	, Shepherding(tying.id())
	, testator_(static_cast<const Testator&>(Soul::of(tying.testator())))
	, novice_(static_cast<const Novice&>(Soul::of(tying.novice())))
{}


Tie::~Tie()
{
	withdraw();
}


const Testator& Tie::testator() const
{
	return testator_;
}


const Novice& Tie::novice() const
{
	return novice_;
}


const Tie& Tie::of(const id::Obedience id)
{
	const auto it = registry().find(id.value());
	if (it == registry().end() || !it->second)
		throw std::invalid_argument("unknown tie");
	return *it->second;
}


void Tie::enroll() const
{
	registry()[obedience_id().value()] = this;
}


void Tie::withdraw() const
{
	auto& ties = registry();
	const auto it = ties.find(obedience_id().value());
	if (it != ties.end() && it->second == this)
		ties.erase(it);
}


} // namespace will::domain
