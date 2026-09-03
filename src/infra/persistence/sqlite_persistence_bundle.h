#pragma once

#include "ports/messenger_persistence.h"

#include "entities/earth.h"
#include "entities/heaven.h"
#include "sqlite_database.h"
#include "sqlite_store.h"
#include "sqlite_letter_repository_impl.h"

#include <string>


namespace will {


/** Owns SQLite database and repository implementations for domain ports. */
class SqlitePersistenceBundle {
public:
    explicit SqlitePersistenceBundle(std::string db_path);

    domain::MessengerPersistence ports();

    domain::LetterRepository& letters();
    domain::Heaven& heaven();
    domain::Earth& earth();

private:
    SqliteDatabase database_;
    SqliteStore store_;
    SqliteLetterRepositoryImpl letters_;
    domain::Heaven heaven_;
    domain::Earth earth_;
};


} // namespace will
