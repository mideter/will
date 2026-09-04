#include "values/device_token.h"

#include <cassert>
#include <cstdlib>


int main()
{
	using namespace will::domain;

	{
		const auto parsed = DeviceToken::parse("abcd1234abcd1234abcd1234abcd1234");
		assert(parsed);
		assert(parsed->text() == "abcd1234abcd1234abcd1234abcd1234");
		assert(*parsed == *DeviceToken::parse("abcd1234abcd1234abcd1234abcd1234"));
	}

	{
		const DeviceToken generated = DeviceToken::generate();
		assert(generated.text().size() == 32);
		assert(DeviceToken::parse(generated.text()));
	}

	assert(!DeviceToken::parse(""));
	assert(!DeviceToken::parse("short"));
	assert(!DeviceToken::parse("zzzzzzzzzzzzzzzzzzzzzzzzzzzzzzzz"));

	return EXIT_SUCCESS;
}
