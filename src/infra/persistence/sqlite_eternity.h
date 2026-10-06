#pragma once

#include "dimensions/eternity.h"
#include "sqlite_space.h"
#include "system_time.h"
#include "sqlite_database.h"

#include <memory>
#include <string>


namespace will {


/// SQLite Eternity — souls, the one Time and Space, indelible words.
/// Opens prefix.eternity.db; realises Spatiality and Temporality over
/// prefix.space.db and prefix.time.db when Creation asks.
class SqliteEternity final : public domain::Eternity {
public:
	explicit SqliteEternity(std::string prefix);

	domain::Time& time() override;
	domain::Space& space() override;
	std::unique_ptr<domain::Spatiality> spatiality(domain::Birth<domain::Creation>) override;
	std::unique_ptr<domain::Temporality> temporality(domain::Birth<domain::Creation>) override;
	domain::matter::Soul enroll(domain::SoulName name) override;
	std::vector<domain::matter::Soul> souls() const override;
	domain::matter::Fatherhood father(domain::id::Soul father, domain::id::Soul child,
									  domain::matter::Fatherhood::Line line) override;
	std::vector<domain::matter::Fatherhood> fatherhoods() const override;
	domain::matter::Word utter(domain::id::Soul author, const domain::Saying& saying) override;
	domain::matter::Word word(domain::id::Word id) const override;
	std::vector<domain::matter::Word> words(const std::vector<domain::id::Word>& ids) const override;

private:
	std::string prefix_;
	mutable SqliteDatabase database_;
	SystemTime time_;
	SqliteSpace space_;
};


} // namespace will
