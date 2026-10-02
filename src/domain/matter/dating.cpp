#include "dating.h"


namespace will::domain::matter {


Dating::Dating(const id::Word id, const Timestamp created_at)
	: id_(id)
	, created_at_(created_at)
{}


} // namespace will::domain::matter
