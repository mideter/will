#pragma once

#include "dimensions/eternity.h"
#include "dimensions/temporality.h"
#include "sqlite_database.h"

#include <string>


namespace will {


/// SQLite Temporality — vessels, embodiments, datings, supplications, executions in time.
/// Owns the time database; the present is taken from Eternity.
class SqliteTemporality final : public domain::Temporality {
public:
	/// Opens the time database at this path.
	SqliteTemporality(std::string path, domain::Eternity& eternity);

	domain::matter::Embodiment embody(domain::id::Soul soul, domain::DeviceToken token) override;
	std::vector<domain::matter::Vessel> vessels() const override;
	std::vector<domain::matter::Embodiment> embodiments() const override;

	domain::matter::Dating date(domain::id::Word id) override;
	std::vector<domain::matter::Dating> datings(const std::vector<domain::id::Word>& ids) const override;

	domain::matter::Supplication ask(domain::id::Soul suppliant, domain::id::Soul addressee) override;
	std::vector<domain::matter::Supplication> supplications(domain::id::Soul addressee) const override;
	void reject(domain::id::Soul suppliant, domain::id::Soul addressee) override;

	domain::matter::Execution execute(domain::id::Word deed, domain::id::Word behest) override;
	std::vector<domain::matter::Execution> executions(const std::vector<domain::id::Word>& words) const override;

private:
	mutable SqliteDatabase time_db_;
	domain::Eternity& eternity_;
};


} // namespace will
