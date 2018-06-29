if (NOT FIPS_IMPORT)
    cmake_minimum_required(VERSION 2.8)
    get_filename_component(FIPS_ROOT_DIR "../fips" ABSOLUTE)
    include("${FIPS_ROOT_DIR}/cmake/fips.cmake")
    fips_setup(PROJECT libsdlang)
endif()

fips_begin_lib(sdlang)
    fips_files(
        sdlang.c
        sdlang.h
        sdlang.inl
    )
fips_end_lib()

if (FIPS_GCC)
    set_target_properties(sdlang PROPERTIES COMPILE_FLAGS "-Wno-implicit-fallthrough")
endif()

if (NOT FIPS_IMPORT)
    include_directories(.)
    fips_begin_app(parser cmdline)
        fips_dir(samples GROUP ".")
        fips_files(
            parser.c
        )
        fips_deps(sdlang)
    fips_end_app()
    fips_finish()
endif()
