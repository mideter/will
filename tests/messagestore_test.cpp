#include "messagestore.h"

#include <cassert>
#include <cstdlib>
#include <string>
#include <unistd.h>


int main()
{
    using namespace will;

    const std::string db_path = "/tmp/will-messagestore-test-" + std::to_string(getpid()) + ".db";
    ::unlink(db_path.c_str());

    MessageStore store(db_path);
    store.insert_message("from-peer", "10.0.0.2", 1000);
    store.insert_message("from-me", "10.0.0.1", 2000);

    const auto rows = store.load_last(10);
    assert(rows.size() == 2);
    assert(rows[0].body == "from-peer");
    assert(rows[0].sender_ip == "10.0.0.2");
    assert(rows[1].body == "from-me");
    assert(rows[1].sender_ip == "10.0.0.1");

    const std::string viewer_ip = "10.0.0.1";
    assert(rows[0].sender_ip != viewer_ip);
    assert(rows[1].sender_ip == viewer_ip);

    ::unlink(db_path.c_str());
    return EXIT_SUCCESS;
}
