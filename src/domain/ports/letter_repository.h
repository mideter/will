#pragma once

#include "entities/letter.h"
#include "ids/abode.h"
#include "ids/god.h"

#include <string_view>
#include <vector>


namespace will::domain {


class LetterRepository {
public:
    virtual ~LetterRepository() = default;

    virtual Letter append(id::Abode abode, id::God author, std::string_view body, Timestamp ts) = 0;
    virtual std::vector<Letter> load_last(id::Abode abode, std::uint32_t limit) = 0;
};


} // namespace will::domain
