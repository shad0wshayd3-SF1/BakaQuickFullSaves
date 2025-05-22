-- set xmake version
set_xmakever("2.9.4")

-- include local folders
includes("lib/commonlibsf")

-- set project
set_project("BakaQuickFullSaves")
set_version("6.0.0")
set_license("GPL-3.0")

-- set defaults
set_languages("c++23")
set_warnings("allextra")

-- add rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- enable lto
set_policy("build.optimization.lto", true)
set_policy("package.requires_lock", true)

-- set config
set_config("commonlib_ini", true)

-- setup targets
target("BakaQuickFullSaves")
    -- bind local dependencies
    add_deps("commonlibsf")

    -- bind package dependencies
    add_packages("figcone")

    -- add commonlibsf plugin
    add_rules("commonlibsf.plugin", {
        name = "BakaQuickFullSaves",
        author = "shad0wshayd3",
        options = {
            no_struct_use = true
        }
    })

    -- add source files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/PCH.h")

    -- add extra files
    add_extrafiles(".clang-format")
