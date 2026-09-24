#pragma once

#include "ports/eternity.h"
#include "system_time.h"
#include "sqlite_database.h"


namespace will {


/// SQLite Eternity — souls, time, and indelible utterances.
class SqliteEternity final : public domain::Eternity {
public:
	explicit SqliteEternity(SqliteDatabase& database);

	domain::Time& time() override;
	domain::id::Soul enroll(domain::SoulName name) override;
	domain::Utterance utter(domain::id::Soul author, const domain::Word& word) override;
	domain::Utterance utterance(domain::id::Letter id) const override;
	std::vector<domain::Utterance> utterances(const std::vector<domain::id::Letter>& ids) const override;

private:
	SqliteDatabase& database_;
	SystemTime time_;
};


} // namespace will
