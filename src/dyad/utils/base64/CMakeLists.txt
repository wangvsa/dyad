set(DYAD_BASE64_SRC ${CMAKE_CURRENT_SOURCE_DIR}/base64.c)
set(DYAD_BASE64_PRIVATE_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/base64.h)
set(DYAD_BASE64_PUBLIC_HEADERS)

add_library(${PROJECT_NAME}_base64 SHARED ${DYAD_BASE64_SRC}
            ${DYAD_BASE64_PRIVATE_HEADERS} ${DYAD_BASE64_PUBLIC_HEADERS})
# set_target_properties(${PROJECT_NAME}_base64 PROPERTIES CMAKE_INSTALL_RPATH
#                       "${DYAD_INSTALL_LIBDIR}")
target_include_directories(${PROJECT_NAME}_base64 PUBLIC
    $<BUILD_INTERFACE:${CMAKE_BINARY_DIR}/src>
    $<INSTALL_INTERFACE:${DYAD_INSTALL_INCLUDEDIR}>)

install(
        TARGETS ${PROJECT_NAME}_base64
        EXPORT ${DYAD_EXPORTED_TARGETS}
        LIBRARY DESTINATION ${DYAD_INSTALL_LIBDIR}
        ARCHIVE DESTINATION ${DYAD_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${DYAD_INSTALL_BINDIR}
)
if(NOT "${DYAD_BASE64_PUBLIC_HEADERS}" STREQUAL "")
    dyad_install_headers("${DYAD_BASE64_PUBLIC_HEADERS}" ${CMAKE_CURRENT_SOURCE_DIR})
endif()
