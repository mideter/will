add_rules("mode.debug", "mode.release")

set_languages("c++20")
set_warnings("all")
set_targetdir("$(projectdir)/build")

add_requires("cli11 2.5.0", "sqlite3")
add_requires("pkgconfig::protobuf")
add_requires("pkgconfig::grpc++")

target("will-domain")
    set_kind("static")
    add_files("src/domain/**.cpp")
    add_files("src/domain/entities/timestamp.cppm", {public = true})
    add_includedirs("src/domain", {public = true})

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
    add_files("src/infra/persistence/**.cpp")
    add_includedirs("src/infra/persistence", {public = true})
    add_packages("sqlite3", {public = true})
    add_deps("will-domain")

target("will-server")
    set_kind("binary")
    add_files("src/server/**.cpp")
    add_includedirs("src/server", "src/server/cli", "src/server/config")
    add_deps("will_proto", "will-persistence")
    add_packages("cli11", "pkgconfig::protobuf", "pkgconfig::grpc++")

target("will-client")
    set_kind("binary")
    add_files("src/client/**.cpp")
    add_includedirs("src/client", "src/client/cli")
    add_deps("will_proto", "will-domain")
    add_packages("cli11", "pkgconfig::protobuf", "pkgconfig::grpc++")

target("will-load-clients")
    set_kind("binary")
    add_files("tools/**.cpp", "src/client/clientconfigvalidator.cpp")
    add_includedirs("src/client", "tools", "tools/cli")
    add_deps("will_proto", "will-domain")
    add_packages("cli11", "pkgconfig::protobuf", "pkgconfig::grpc++")

includes("tests")
