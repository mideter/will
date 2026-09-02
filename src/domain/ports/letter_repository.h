#pragma once

#include "entities/letter.h"
#include "ids/abode_id.h"
#include "ids/god_id.h"

#include <string_view>
#include <vector>


namespace will::domain {


class LetterRepository {
public:
    virtual ~LetterRepository() = default;

    virtual Letter append(AbodeId abode, GodId author, std::string_view body, Timestamp ts) = 0;
    virtual std::vector<Letter> load_last(AbodeId abode, std::uint32_t limit) = 0;
};


} // namespace will::domain
