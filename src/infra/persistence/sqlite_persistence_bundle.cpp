#include "sqlite_persistence_bundle.h"

#include <string>


namespace will {
namespace {


std::string face_path(std::string prefix, const char* face)
{
	if (prefix.size() > 3 && prefix.ends_with(".db"))
		prefix.resize(prefix.size() - 3);
	return prefix + "." + face + ".db";
}


} // namespace


SqlitePersistenceBundle::SqlitePersistenceBundle(std::string prefix)
	: eternity_db_(face_path(prefix, "eternity"), SqliteFace::Eternity)
	, space_db_(face_path(prefix, "space"), SqliteFace::Spatiality)
	, time_db_(face_path(prefix, "time"), SqliteFace::Temporality)
	, eternity_(eternity_db_)
	, spatiality_(space_db_)
	, temporality_(time_db_, eternity_)
	, creation_(eternity_, spatiality_, temporality_)
{}


domain::World& SqlitePersistenceBundle::world()
{
	return creation_.world();
}


} // namespace will
