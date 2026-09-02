#include "entities/letter.h"
#include "ids/letter_id.h"

#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <string>


int main()
{
    using namespace will::domain;

    {
        const LetterId id{1};
        assert(id.value() == 1);
    }

    try {
        LetterId{0};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    {
        const Letter letter{LetterId{1}, AbodeId::global(), GodId{7}, "hello", Timestamp{100}};
        assert(letter.id() == LetterId{1});
        assert(letter.abode_id() == AbodeId::global());
        assert(letter.author_id() == GodId{7});
        assert(letter.body() == "hello");
        assert(letter.created_at() == Timestamp{100});
    }

    try {
        Letter{LetterId{1}, AbodeId::global(), GodId{0}, "x", Timestamp{0}};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    try {
        Letter{LetterId{1}, AbodeId::global(), GodId{1}, "", Timestamp{0}};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    try {
        Letter{LetterId{1}, AbodeId::global(), GodId{1}, std::string(Letter::MaxBodyLength + 1, 'a'),
               Timestamp{0}};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    {
        const Letter max_body{LetterId{1}, AbodeId::global(), GodId{1},
                              std::string(Letter::MaxBodyLength, 'a'), Timestamp{0}};
        assert(max_body.body().size() == Letter::MaxBodyLength);
    }

    return EXIT_SUCCESS;
}
