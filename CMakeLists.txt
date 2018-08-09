if (NOT FIPS_IMPORT)
    cmake_minimum_required(VERSION 2.8)
    get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
    include("${FIPS_ROOT_DIR}/cmake/fips.cmake")
    fips_setup(PROJECT libsdlang)
endif()

fips_begin_lib(sdlang)
    fips_vs_warning_level(3)
    fips_files(
        sdlang.c
        sdlang.h
        sdlang.inl
    )
fips_end_lib()

if (FIPS_MSVC)
    target_compile_definitions(sdlang PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()

if (FIPS_GCC)
    target_compile_options(sdlang PRIVATE "-Wno-implicit-fallthrough")
endif()

if (NOT FIPS_IMPORT)
    include_directories(.)
    fips_begin_app(parser cmdline)
        fips_vs_warning_level(3)
        fips_dir(samples GROUP ".")
        fips_files(
            parser.c
        )
        fips_deps(sdlang)
    fips_end_app()
    fips_finish()
    if (FIPS_MSVC)
        target_compile_definitions(parser PRIVATE _CRT_SECURE_NO_WARNINGS)
    endif()
endif()
