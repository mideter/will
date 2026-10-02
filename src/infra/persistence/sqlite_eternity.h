#pragma once

#include "ports/eternity.h"
#include "sqlite_space.h"
#include "system_time.h"
#include "sqlite_database.h"


namespace will {


/// SQLite Eternity — souls, the one Time and Space, indelible utterances.
class SqliteEternity final : public domain::Eternity {
public:
	explicit SqliteEternity(SqliteDatabase& database);

	domain::Time& time() override;
	domain::Space& space() override;
	domain::matter::Soul enroll(domain::SoulName name) override;
	std::vector<domain::matter::Soul> souls() const override;
	domain::matter::Utterance utter(domain::id::Soul author, const domain::Saying& saying) override;
	domain::matter::Utterance utterance(domain::id::Word id) const override;
	std::vector<domain::matter::Utterance> utterances(const std::vector<domain::id::Word>& ids) const override;

private:
	SqliteDatabase& database_;
	SystemTime time_;
	SqliteSpace space_;
};


} // namespace will
