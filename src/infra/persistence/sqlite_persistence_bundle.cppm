module;

#include <string>

export module will.persistence.sqlite_persistence_bundle;

export import will.persistence.sqlite_database;
export import will.persistence.sqlite_message_repository;
export import will.persistence.sqlite_user_repository;
export import will.domain.messenger_persistence;

export namespace will {


/** Owns SQLite database and repository implementations for domain ports. */
class SqlitePersistenceBundle {
public:
    explicit SqlitePersistenceBundle(std::string db_path);

    domain::MessengerPersistence ports();

    domain::MessageRepository& messages();
    domain::UserRepository& users();

private:
    SqliteDatabase database_;
    SqliteMessageRepositoryImpl messages_;
    SqliteUserRepositoryImpl users_;
};


} // namespace will
