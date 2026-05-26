option(ENABLE_DSPACES_TEST "Enable DataSpaces perf test" OFF)
option(ENABLE_UNIT_TEST "Enable DYAD unit tests" ON)
option(ENABLE_SHUFFLE_TEST "Enable DYAD data shuffle tests" OFF)

if (ENABLE_DSPACES_TEST)
    add_subdirectory(dspaces_perf)
endif ()

if (ENABLE_UNIT_TEST)
    add_subdirectory(unit)
endif ()

if (ENABLE_SHUFFLE_TEST)
    add_subdirectory(shuffle)
endif ()
