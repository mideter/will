add_rules("mode.debug", "mode.release")

set_languages("c++20")
set_warnings("all")
set_targetdir("$(projectdir)/build")

add_requires("asio 1.30.2", "cli11 2.5.0")
add_requires("openssl", "sqlite3")

target("will-domain")
    set_kind("static")
    add_files(
        "src/domain/support/device_token.cpp",
        "src/domain/support/user_name.cpp",
        "src/domain/usecases/send_chat_message.cpp",
        "src/domain/usecases/fetch_chat_history.cpp",
        "src/domain/usecases/authenticate_device.cpp")
    add_includedirs("src/domain", {public = true})

target("will-protocol")
    set_kind("static")
    add_files(
        "src/infra/protocol/tcpframe.cpp",
        "src/infra/protocol/wiremessage_codec.cpp",
        "src/infra/protocol/wiremessage_codec_internal.cpp",
        "src/infra/protocol/wiremessage_messages.cpp")
    add_includedirs("src/infra/protocol", {public = true})
    add_deps("will-domain")

target("will-transport")
    set_kind("static")
    add_files(
        "src/infra/transport/tcpframedchannel.cpp",
        "src/infra/transport/tcpframedchannel_state.cpp",
        "src/infra/transport/tcpframereader.cpp",
        "src/infra/transport/tcpframewriter.cpp",
        "src/infra/transport/tcpstreamsocket.cpp")
    add_includedirs("src/infra/transport", {public = true})
    add_defines("ASIO_STANDALONE", {public = true})
    add_packages("asio", {public = true})
    add_syslinks("pthread")
    add_deps("will-protocol")

target("will-server")
    set_kind("binary")
    add_files(
        "src/server/server.cpp",
        "src/server/cli/servercliapp.cpp",
        "src/server/config/serverconfigparser.cpp",
        "src/server/tcpconnectionregistry.cpp",
        "src/server/config/serverconfigvalidator.cpp",
        "src/server/willserver.cpp",
        "src/server/asioserver.cpp",
        "src/server/iocontext.cpp",
        "src/server/iocontextthreadpool.cpp",
        "src/server/tcpconnection.cpp",
        "src/server/connectionaccountstore.cpp",
        "src/server/inbound_client_message_handler.cpp",
        "src/server/protocoladapter.cpp",
        "src/server/tcpconnectionparticipantnotifierimpl.cpp",
        "src/infra/persistence/sqlite_database.cpp",
        "src/infra/persistence/password_hash.cpp",
        "src/infra/persistence/sqlite_message_repository_impl.cpp",
        "src/infra/persistence/sqlite_user_repository_impl.cpp",
        "src/infra/persistence/sqlite_persistence_bundle.cpp")
    add_includedirs(
        "src/server",
        "src/server/cli",
        "src/server/config",
        "src/domain",
        "src/infra/persistence")
    add_defines("ASIO_STANDALONE")
    add_deps("will-domain", "will-protocol", "will-transport")
    add_packages("asio", "cli11", "openssl", "sqlite3")
    add_syslinks("pthread")

target("will-client")
    set_kind("binary")
    add_files(
        "src/client/client.cpp",
        "src/client/consoleui.cpp",
        "src/client/inbound_server_message_handler.cpp",
        "src/client/chatsession.cpp",
        "src/client/willclient.cpp",
        "src/client/devicetokenstore.cpp",
        "src/client/clientconfigvalidator.cpp",
        "src/client/cli/clientcliapp.cpp",
        "src/client/cli/clientconfigparser.cpp")
    add_includedirs(
        "src/client",
        "src/client/cli",
        "src/domain")
    add_defines("ASIO_STANDALONE")
    add_deps("will-domain", "will-protocol", "will-transport")
    add_packages("asio", "cli11")
    add_syslinks("pthread")

target("will-load-clients")
    set_kind("binary")
    add_files(
        "tools/load_clients.cpp",
        "tools/cli/loadclientscliapp.cpp",
        "tools/cli/loadclientsconfigparser.cpp",
        "src/client/clientconfigvalidator.cpp")
    add_includedirs(
        "src/client",
        "src/domain",
        "tools",
        "tools/cli")
    add_defines("ASIO_STANDALONE")
    add_deps("will-domain", "will-protocol")
    add_packages("asio", "cli11")
    add_syslinks("pthread")

includes("tests")
