# --------------------------------------------------------------- #
#  dyad_cache_client.so
# --------------------------------------------------------------- #
set(CACHE_CLIENT_SRC ${CMAKE_CURRENT_SOURCE_DIR}/cache_margo_client.c
                     ${CMAKE_CURRENT_SOURCE_DIR}/cache_client.c)

set(CACHE_CLIENT_PRIVATE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/cache_margo_client.h
                                 ${CMAKE_CURRENT_SOURCE_DIR}/cache_common.h)
set(CACHE_CLIENT_PUBLIC_HEADERS)

# for now this dyad_cache_client.so is only used by my
# example client code (executable) below. but it seems
# linking this library always results in a json-c error,
# while directly compile with the code works well.
add_library(${PROJECT_NAME}_cache_client SHARED ${CACHE_CLIENT_SRC} ${CACHE_CLIENT_PUBLIC_HEADERS} ${CACHE_CLIENT_PRIVATE_HEADERS})

target_link_libraries(${PROJECT_NAME}_cache_client PRIVATE PkgConfig::MARGO json-c::json-c)

target_compile_definitions(${PROJECT_NAME}_cache_client PUBLIC DYAD_HAS_CONFIG)

target_include_directories(${PROJECT_NAME}_cache_client PUBLIC
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>
    $<INSTALL_INTERFACE:${DYAD_INSTALL_INCLUDEDIR}>)

dyad_add_werror_if_needed(${PROJECT_NAME}_cache_client)

install(
    TARGETS ${PROJECT_NAME}_cache_client
    EXPORT ${DYAD_EXPORTED_TARGETS}
    LIBRARY DESTINATION ${DYAD_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${DYAD_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${DYAD_INSTALL_BINDIR}
)

if(NOT ${CACHE_CLIENT_PUBLIC_HEADERS} STREQUAL "")
    dyad_install_headers("${CACHE_CLIENT_PUBLIC_HEADERS}" ${CMAKE_CURRENT_SOURCE_DIR})
endif()


# --------------------------------------------------------------- #
#  cache_server executable
# --------------------------------------------------------------- #
set(CACHE_SERVER "cache_server")
set(CACHE_SERVER_SRC ${CMAKE_CURRENT_SOURCE_DIR}/cache_margo_server.c
                         ${CMAKE_CURRENT_SOURCE_DIR}/cache_server.c)

set(CACHE_SERVER_PRIVATE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/cache_margo_server.h
                                 ${CMAKE_CURRENT_SOURCE_DIR}/cache_common.h)
set(CACHE_SERVER_PUBLIC_HEADERS)

add_executable(${CACHE_SERVER} ${CACHE_SERVER_SRC})
target_link_libraries(${CACHE_SERVER} PRIVATE PkgConfig::MARGO json-c::json-c)

install(
    TARGETS ${CACHE_SERVER}
    EXPORT  ${DYAD_EXPORTED_TARGETS}
    LIBRARY DESTINATION ${DYAD_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${DYAD_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${DYAD_INSTALL_BINDIR}
)


# --------------------------------------------------------------- #
#  cache_server executable
# --------------------------------------------------------------- #
set(EXAMPLE_CLIENT_EXE "example_cache_client")
set(EXAMPLE_CLIENT_EXE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/example_cache_client.c)

add_executable(${EXAMPLE_CLIENT_EXE} ${EXAMPLE_CLIENT_EXE_SRC})
target_link_libraries(${EXAMPLE_CLIENT_EXE} PRIVATE PkgConfig::MARGO json-c::json-c ${PROJECT_NAME}_cache_client)
#target_link_libraries(${EXAMPLE_CLIENT_EXE} PRIVATE PkgConfig::MARGO json-c::json-c)

install(
    TARGETS ${EXAMPLE_CLIENT_EXE}
    EXPORT  ${DYAD_EXPORTED_TARGETS}
    LIBRARY DESTINATION ${DYAD_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${DYAD_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${DYAD_INSTALL_BINDIR}
)
