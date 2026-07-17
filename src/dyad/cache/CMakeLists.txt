# Cache-eviction policy interface
set(CACHE_SRC ${CMAKE_CURRENT_SOURCE_DIR}/dyad_cache_api.c
              ${CMAKE_CURRENT_SOURCE_DIR}/dyad_cache.c
              ${CMAKE_CURRENT_SOURCE_DIR}/lru_cache.c
              ${CMAKE_CURRENT_SOURCE_DIR}/fifo_cache.c)
set(CACHE_PRIVATE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/dyad_cache_api.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/dyad_cache_int.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/lru_cache.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/fifo_cache.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/../common/dyad_structures_int.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/dyad/common/dyad_cache.h
                          ${CMAKE_CURRENT_SOURCE_DIR}/../../../include/dyad/common/dyad_rc.h)
set(CACHE_PUBLIC_HEADERS)

# Unlike the DTL backends, LRU and FIFO have no external dependencies, so
# both are always compiled in -- no compile-time feature flags needed.

add_library(${PROJECT_NAME}_cache SHARED ${CACHE_SRC} ${CACHE_PUBLIC_HEADERS} ${CACHE_PRIVATE_HEADERS})
target_link_libraries(${PROJECT_NAME}_cache PRIVATE ${PROJECT_NAME}_utils)

target_compile_definitions(${PROJECT_NAME}_cache PUBLIC DYAD_HAS_CONFIG)
target_include_directories(${PROJECT_NAME}_cache PUBLIC
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>
    $<INSTALL_INTERFACE:${DYAD_INSTALL_INCLUDEDIR}>)

dyad_add_werror_if_needed(${PROJECT_NAME}_cache)
if(DYAD_LOGGER STREQUAL "CPP_LOGGER")
    target_link_libraries(${PROJECT_NAME}_cache PRIVATE ${cpp-logger_LIBRARIES})
endif()
if(DYAD_PROFILER STREQUAL "DFTRACER")
    target_link_libraries(${PROJECT_NAME}_cache PRIVATE ${DFTRACER_LIBRARIES})
endif()

install(
        TARGETS ${PROJECT_NAME}_cache
        EXPORT ${DYAD_EXPORTED_TARGETS}
        LIBRARY DESTINATION ${DYAD_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${DYAD_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${DYAD_INSTALL_BINDIR}
)
if(NOT ${CACHE_PUBLIC_HEADERS} STREQUAL "")
    dyad_install_headers("${CACHE_PUBLIC_HEADERS}" ${CMAKE_CURRENT_SOURCE_DIR})
endif()
