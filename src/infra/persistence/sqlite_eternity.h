#pragma once

#include "dimensions/eternity.h"
#include "sqlite_space.h"
#include "system_time.h"
#include "sqlite_database.h"


namespace will {


/// SQLite Eternity — souls, the one Time and Space, indelible words.
class SqliteEternity final : public domain::Eternity {
public:
	explicit SqliteEternity(SqliteDatabase& database);

	domain::Time& time() override;
	domain::Space& space() override;
	domain::matter::Soul enroll(domain::SoulName name) override;
	std::vector<domain::matter::Soul> souls() const override;
	domain::matter::Word utter(domain::id::Soul author, const domain::Saying& saying) override;
	domain::matter::Word word(domain::id::Word id) const override;
	std::vector<domain::matter::Word> words(const std::vector<domain::id::Word>& ids) const override;

private:
	SqliteDatabase& database_;
	SystemTime time_;
	SqliteSpace space_;
};


} // namespace will
