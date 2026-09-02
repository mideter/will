#pragma once

#include "ports/messenger_persistence.h"

#include "sqlite_database.h"
#include "sqlite_letter_repository_impl.h"
#include "sqlite_user_repository_impl.h"

#include <string>


namespace will {


/** Owns SQLite database and repository implementations for domain ports. */
class SqlitePersistenceBundle {
public:
    explicit SqlitePersistenceBundle(std::string db_path);

    domain::MessengerPersistence ports();

    domain::LetterRepository& letters();
    domain::UserRepository& users();

private:
    SqliteDatabase database_;
    SqliteLetterRepositoryImpl letters_;
    SqliteUserRepositoryImpl users_;
};


} // namespace will
