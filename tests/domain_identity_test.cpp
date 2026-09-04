#include "identity/abode.h"
#include "identity/letter.h"
#include "identity/man.h"
#include "identity/soul.h"
#include "identity/vessel.h"

#include <cassert>
#include <cstdlib>
#include <stdexcept>


int main()
{
	using namespace will::domain;

	{
		const id::Soul id{42};
		assert(id.value() == 42);
	}

	try {
		id::Soul{0};
		return EXIT_FAILURE;
	} catch (const std::invalid_argument&) {
	}

	{
		const id::Letter id{7};
		assert(id.value() == 7);
	}

	try {
		id::Letter{0};
		return EXIT_FAILURE;
	} catch (const std::invalid_argument&) {
	}

	{
		assert(id::Abode::global() == id::Abode{1});
	}

	try {
		id::Abode{0};
		return EXIT_FAILURE;
	} catch (const std::invalid_argument&) {
	}

	try {
		id::Vessel{0};
		return EXIT_FAILURE;
	} catch (const std::invalid_argument&) {
	}

	try {
		id::Man{0};
		return EXIT_FAILURE;
	} catch (const std::invalid_argument&) {
	}

	{
		const id::Man id{3};
		assert(id.value() == 3);
	}

	{
		const id::Abode abode{5};
		assert(abode.value() == 5);
		assert(abode != id::Abode::global());
	}

	return EXIT_SUCCESS;
}
