#pragma once

#include <stdexcept>
#include <string>


namespace will::domain {


/// Refusal (Отказ) — the world does not let a man do what he would: it goes against
/// its rules, not against the code. It names its reason, that whoever tells the man of
/// it says why in his own words; the text is the reason as the world says it.
/// Everything else thrown as a logic_error is a breach of the world's own order — a
/// fault of the code, never a man's doing.
class Refusal : public std::logic_error {
public:
	enum class Reason {
		// At the gates.
		AdmitOnlyAtGates,
		KeepGatesByKind,
		AlreadyDwells,
		NotAtGates,
		BearOnlyAtGates,
		BearOnlyAtKeptGates,
		NoSuchUnborn,
		// In the abode.
		DoesNotDwell,
		DoesNotEnterRoom,
		RegardOnlyInUpperRoom,
		ArrangeOnlyInUpperRoom,
		FatherByFleshNeighbour,
		FatherBySpiritFriend,
		OnlyHostWrites,
		SayOnlyInWord,
		// Ties.
		CannotSupplicateOneself,
		SupplicateOnlyDweller,
		HearOnlyDweller,
		AlreadyTied,
		TieOnlyMutualDwellers,
		UnknownSupplication,
		WillOnlyInContemplatedTie,
		OnlyTestatorWills,
		FulfilOnlyInContemplatedTie,
		OnlyNoviceFulfils,
		AlreadyFulfilled,
		PlainBehestWithoutExercises,
		// Trainings.
		TrainOnlyInContemplatedTie,
		OnlyNoviceTrains,
		NoSuchExercise,
		NoSuchApproach,
		ApproachAlreadyDone,
		ApproachesInOrder,
		AnotherApproachUnderway,
		NoApproachUnderway,
		FinishApproachFirst,
		// Fatherhood.
		NotOwnFather,
		NotDescendantAsFather,
	};

	Refusal(const Reason reason, const std::string& said)
		: std::logic_error(said)
		, reason_(reason)
	{}

	Reason reason() const noexcept { return reason_; }

private:
	Reason reason_;
};


} // namespace will::domain
