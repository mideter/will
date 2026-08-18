add_rules("mode.debug", "mode.release")

set_languages("c++20")
set_warnings("all")
set_targetdir("$(projectdir)/build")
set_policy("build.c++.modules.gcc.cxx11abi", true)

add_requires("cli11 2.5.0", "sqlite3")
add_requires("pkgconfig::protobuf")
add_requires("pkgconfig::grpc++")

target("will-domain")
    set_kind("static")
    set_policy("build.c++.modules", true)
    add_files("src/domain/**.cpp")
    add_files("src/domain/**.cppm", {public = true})

target("will_proto")
    set_kind("static")
    add_packages("pkgconfig::protobuf", "pkgconfig::grpc++")
    add_rules("protobuf.cpp")
    add_files("proto/messenger.proto", {
        proto_rootdir = ".",
        proto_public = true,
        proto_grpc_cpp_plugin = true
    })

target("will-persistence")
    set_kind("static")
    set_policy("build.c++.modules", true)
    add_files("src/infra/persistence/**.cpp")
    add_files("src/infra/persistence/**.cppm", {public = true})
    add_packages("sqlite3", {public = true})
    add_deps("will-domain")

target("will-server-lib")
    set_kind("static")
    set_policy("build.c++.modules", true)
    add_files("src/server/**.cpp|server.cpp|cli/servercliapp.cpp")
    add_files("src/server/cli/servercliapp.cpp", {force = {cxflags = "-O0"}})
    add_files("src/server/**.cppm", {public = true})
    add_deps("will_proto", "will-persistence")
    add_packages("cli11", "pkgconfig::protobuf", "pkgconfig::grpc++", {public = true})

target("will-server")
    set_kind("binary")
    set_policy("build.c++.modules", true)
    add_files("src/server/server.cpp", {force = {cxflags = "-O0"}})
    add_deps("will-server-lib")
    add_packages("cli11", "pkgconfig::protobuf", "pkgconfig::grpc++")

target("will-client-lib")
    set_kind("static")
    set_policy("build.c++.modules", true)
    add_files("src/client/clientconfigvalidator.cpp")
    add_files("src/client/consoleui.cpp")
    add_files("src/client/devicetokenstore.cpp")
    add_files("src/client/**.cppm", {public = true})
    add_deps("will_proto", "will-domain")
    add_packages("cli11", "pkgconfig::protobuf", "pkgconfig::grpc++", {public = true})

target("will-client")
    set_kind("binary")
    set_policy("build.c++.modules", true)
    add_files("src/client/willclient.cpp", {
        force = {cxflags = "-O0 -fvisibility=default -fno-visibility-inlines-hidden"}
    })
    add_files("src/client/grpc_stream_inst.cpp")
    add_deps("will-client-lib", "will_proto")
    add_packages("cli11", "pkgconfig::protobuf", "pkgconfig::grpc++")

target("will-load-clients")
    set_kind("binary")
    set_policy("build.c++.modules", true)
    add_files("tools/load_clients.cpp", {force = {cxflags = "-O0"}})
    add_files("tools/**.cppm", {public = true})
    add_deps("will_proto", "will-client-lib")
    add_packages("cli11", "pkgconfig::protobuf", "pkgconfig::grpc++")

includes("tests")
