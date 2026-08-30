-- include subprojects
includes("lib/commonlibsf")

-- set project constants
set_project("BakaQuickFullSaves")
set_version("7.0.0")
set_license("GPL-3.0")
set_languages("c++23")
set_warnings("allextra")

-- add common rules
add_rules("mode.debug", "mode.releasedbg")
add_rules("plugin.vsxmake.autoupdate")

-- set configs
set_config("commonlib_ini", true)

-- define targets
target("BakaQuickFullSaves")
    add_rules("commonlibsf.plugin", {
        name = "BakaQuickFullSaves",
        author = "shad0wshayd3",
        options = {
            no_struct_use = true
        }
    })

    -- add src files
    add_files("src/**.cpp")
    add_headerfiles("src/**.h")
    add_includedirs("src")
    set_pcxxheader("src/pch.h")

    -- add extra files
    add_extrafiles(".clang-format")
