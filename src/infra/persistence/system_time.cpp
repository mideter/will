#include "system_time.h"

#include <chrono>


namespace will {


domain::Timestamp SystemTime::instant() const
{
	using std::chrono::system_clock;
	using std::chrono::duration_cast;
	using std::chrono::nanoseconds;

	const auto tse = system_clock::now().time_since_epoch();
	return domain::Timestamp{duration_cast<nanoseconds>(tse).count()};
}


} // namespace will
