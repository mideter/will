local function test_target(name)
    target(name)
        set_kind("binary")
        set_default(false)
        add_tests("default")
end

local function with_run_dep(dep_name)
    on_config(function (target)
        target:set("runargs", target:dep(dep_name):targetfile())
    end)
end

test_target("timestamp-test")
    add_files("timestamp_test.cpp")
    add_deps("will-domain")
    set_policy("build.c++.modules", true)

test_target("device-token-test")
    add_files("device_token_test.cpp")
    add_deps("will-domain")

test_target("user-name-test")
    add_files("user_name_test.cpp")
    add_deps("will-domain")

test_target("will-server-config-test")
    add_files(
        "server_config_test.cpp",
        "$(projectdir)/src/server/config/serverconfigvalidator.cpp")
    add_includedirs("$(projectdir)/src/server/config")

test_target("will-cli-test")
    add_files(
        "cli_test.cpp",
        "$(projectdir)/src/server/cli/servercliapp.cpp")
    add_includedirs(
        "$(projectdir)/src/server/config",
        "$(projectdir)/src/server/cli")
    add_packages("cli11")

test_target("will-client-config-test")
    add_files(
        "client_config_test.cpp",
        "$(projectdir)/src/client/clientconfigvalidator.cpp")
    add_includedirs("$(projectdir)/src/client")
    add_deps("will-domain")

test_target("will-client-cli-test")
    add_files(
        "client_cli_test.cpp",
        "$(projectdir)/src/client/cli/clientcliapp.cpp")
    add_includedirs(
        "$(projectdir)/src/client",
        "$(projectdir)/src/client/cli")
    add_packages("cli11")

test_target("will-client-cli-integration-test")
    add_files("client_cli_integration_test.cpp")
    add_deps("will-client")
    add_packages("cli11")
    with_run_dep("will-client")

test_target("will-server-cli-integration-test")
    add_files("server_cli_integration_test.cpp")
    add_deps("will-server")
    add_packages("cli11")
    with_run_dep("will-server")

test_target("will-history-integration-test")
    add_files("history_integration_test.cpp")
    add_deps("will-server", "will_proto")
    add_packages("pkgconfig::protobuf", "pkgconfig::grpc++")
    with_run_dep("will-server")

test_target("will-connection-account-store-test")
    add_files(
        "connection_account_store_test.cpp",
        "$(projectdir)/src/server/connectionaccountstore.cpp")
    add_includedirs("$(projectdir)/src/server")
    add_deps("will-domain")

test_target("will-session-takeover-integration-test")
    add_files("session_takeover_integration_test.cpp")
    add_deps("will-server", "will_proto")
    add_packages("pkgconfig::protobuf", "pkgconfig::grpc++")
    with_run_dep("will-server")

test_target("will-domain-test")
    add_files("domain_test.cpp")
    add_includedirs("$(projectdir)/tests")
    add_deps("will-domain")

test_target("will-sqlite-persistence-test")
    add_files("sqlite_persistence_test.cpp")
    add_deps("will-persistence")
