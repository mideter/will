target("wire-message-test")
    set_kind("binary")
    set_default(false)
    add_files("wire_message_test.cpp")
    add_deps("will-protocol")
    add_tests("default")

target("device-token-test")
    set_kind("binary")
    set_default(false)
    add_files("device_token_test.cpp")
    add_includedirs("$(projectdir)/src/domain")
    add_deps("will-domain")
    add_tests("default")

target("user-name-test")
    set_kind("binary")
    set_default(false)
    add_files("user_name_test.cpp")
    add_includedirs("$(projectdir)/src/domain")
    add_deps("will-domain")
    add_tests("default")

target("tcp-framed-channel-test")
    set_kind("binary")
    set_default(false)
    add_files("tcp_framed_channel_test.cpp")
    add_deps("will-transport")
    add_tests("default")

target("will-server-config-test")
    set_kind("binary")
    set_default(false)
    add_files(
        "server_config_test.cpp",
        "$(projectdir)/src/server/config/serverconfigvalidator.cpp")
    add_includedirs("$(projectdir)/src/server/config")
    add_tests("default")

target("will-cli-test")
    set_kind("binary")
    set_default(false)
    add_files(
        "cli_test.cpp",
        "$(projectdir)/src/server/cli/servercliapp.cpp")
    add_includedirs(
        "$(projectdir)/src/server/config",
        "$(projectdir)/src/server/cli")
    add_deps("will-protocol")
    add_packages("cli11")
    add_tests("default")

target("will-client-config-test")
    set_kind("binary")
    set_default(false)
    add_files(
        "client_config_test.cpp",
        "$(projectdir)/src/client/clientconfigvalidator.cpp")
    add_includedirs(
        "$(projectdir)/src/client",
        "$(projectdir)/src/domain")
    add_defines("ASIO_STANDALONE")
    add_deps("will-domain")
    add_packages("asio")
    add_tests("default")

target("will-client-cli-test")
    set_kind("binary")
    set_default(false)
    add_files(
        "client_cli_test.cpp",
        "$(projectdir)/src/client/cli/clientcliapp.cpp")
    add_includedirs(
        "$(projectdir)/src/client",
        "$(projectdir)/src/client/cli")
    add_deps("will-protocol")
    add_packages("cli11")
    add_tests("default")

target("will-client-cli-integration-test")
    set_kind("binary")
    set_default(false)
    add_files("client_cli_integration_test.cpp")
    add_deps("will-client")
    add_packages("cli11")
    add_tests("default")
    on_config(function (target)
        local dep = target:dep("will-client")
        target:set("runargs", dep:targetfile())
    end)

target("will-server-cli-integration-test")
    set_kind("binary")
    set_default(false)
    add_files("server_cli_integration_test.cpp")
    add_deps("will-server")
    add_packages("cli11")
    add_tests("default")
    on_config(function (target)
        local dep = target:dep("will-server")
        target:set("runargs", dep:targetfile())
    end)

target("will-history-integration-test")
    set_kind("binary")
    set_default(false)
    add_files("history_integration_test.cpp")
    add_deps("will-server", "will-protocol")
    add_tests("default")
    on_config(function (target)
        local dep = target:dep("will-server")
        target:set("runargs", dep:targetfile())
    end)

target("will-connection-account-store-test")
    set_kind("binary")
    set_default(false)
    add_files(
        "connection_account_store_test.cpp",
        "$(projectdir)/src/server/connectionaccountstore.cpp")
    add_includedirs(
        "$(projectdir)/src/server",
        "$(projectdir)/src/domain")
    add_deps("will-domain")
    add_tests("default")

target("will-session-takeover-integration-test")
    set_kind("binary")
    set_default(false)
    add_files("session_takeover_integration_test.cpp")
    add_deps("will-server", "will-protocol")
    add_tests("default")
    on_config(function (target)
        local dep = target:dep("will-server")
        target:set("runargs", dep:targetfile())
    end)

target("will-heartbeat-integration-test")
    set_kind("binary")
    set_default(false)
    add_files("heartbeat_integration_test.cpp")
    add_deps("will-server", "will-protocol")
    add_tests("default")
    on_config(function (target)
        local dep = target:dep("will-server")
        target:set("runargs", dep:targetfile())
    end)

target("will-domain-test")
    set_kind("binary")
    set_default(false)
    add_files("domain_test.cpp")
    add_includedirs(
        "$(projectdir)/src/domain",
        "$(projectdir)/tests")
    add_deps("will-domain")
    add_tests("default")

target("will-sqlite-persistence-test")
    set_kind("binary")
    set_default(false)
    add_files(
        "sqlite_persistence_test.cpp",
        "$(projectdir)/src/infra/persistence/sqlite_database.cpp",
        "$(projectdir)/src/infra/persistence/password_hash.cpp",
        "$(projectdir)/src/infra/persistence/sqlite_message_repository_impl.cpp",
        "$(projectdir)/src/infra/persistence/sqlite_user_repository_impl.cpp")
    add_includedirs(
        "$(projectdir)/src/infra/persistence",
        "$(projectdir)/src/domain")
    add_deps("will-domain")
    add_packages("openssl", "sqlite3")
    add_tests("default")
