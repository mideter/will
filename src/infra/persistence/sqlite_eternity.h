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
	domain::id::Soul enroll(domain::SoulName name) override;
	domain::Utterance utter(domain::id::Soul author, const domain::Word& word) override;
	domain::Utterance utterance(domain::id::Letter id) const override;
	std::vector<domain::Utterance> utterances(const std::vector<domain::id::Letter>& ids) const override;

private:
	SqliteDatabase& database_;
	SystemTime time_;
	SqliteSpace space_;
};


} // namespace will
