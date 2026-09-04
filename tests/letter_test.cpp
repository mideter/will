#include "entities/letter.h"
#include "ids/letter.h"

#include <cassert>
#include <cstdlib>
#include <stdexcept>
#include <string>


int main()
{
    using namespace will::domain;

    {
        const id::Letter id{1};
        assert(id.value() == 1);
    }

    try {
        id::Letter{0};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    {
        const Letter letter{id::Letter{1}, id::Abode::global(), id::Soul{7}, "hello", Timestamp{100}};
        assert(letter.id() == id::Letter{1});
        assert(letter.abode_id() == id::Abode::global());
        assert(letter.author_id() == id::Soul{7});
        assert(letter.body() == "hello");
        assert(letter.created_at() == Timestamp{100});
    }

    try {
        Letter{id::Letter{1}, id::Abode::global(), id::Soul{0}, "x", Timestamp{0}};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    try {
        Letter{id::Letter{1}, id::Abode::global(), id::Soul{1}, "", Timestamp{0}};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    try {
        Letter{id::Letter{1}, id::Abode::global(), id::Soul{1}, std::string(Letter::MaxBodyLength + 1, 'a'),
               Timestamp{0}};
        return EXIT_FAILURE;
    } catch (const std::invalid_argument&) {
    }

    {
        const Letter max_body{id::Letter{1}, id::Abode::global(), id::Soul{1},
                              std::string(Letter::MaxBodyLength, 'a'), Timestamp{0}};
        assert(max_body.body().size() == Letter::MaxBodyLength);
    }

    return EXIT_SUCCESS;
}
