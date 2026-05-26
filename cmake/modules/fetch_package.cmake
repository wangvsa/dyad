# fetch_package.cmake
#
# A CMake helper function to find or fetch an external package.
#
# Usage:
#   fetch_package(
#       NAME     <name1> [name2 ...]      # required: candidate package names (first is primary)
#                                         # multiple names handle different casing conventions
#                                         # e.g. NAME gotcha GOTCHA Gotcha
#       GIT      <url>                    # required (mutually exclusive with URL):
#                                         # git repository URL
#         or
#       URL      <url>                    # required (mutually exclusive with GIT):
#                                         # archive download URL
#       [TAG     <tag>]                   # recommended with GIT: tag, branch, or commit hash
#       [URL_HASH <hash>]                 # recommended with URL: integrity hash e.g. SHA256=abc...
#       [VERSION  <version>]              # optional: CMake package version for find_package
#                                         # must match installed package's reported version
#                                         # e.g. VERSION 2.4.2 (not the git tag)
#       [SUBMODULES <path> [path2 ...]]   # optional, GIT only: git submodules to initialize
#                                         #   omitted  -> disable all submodules (default)
#                                         #   ""       -> explicitly disable all submodules
#                                         #   path ... -> initialize only listed submodules
#                                         #               with GIT_SUBMODULES_RECURSE ON
#       [PATCH_COMMAND <cmd> [args ...]]  # optional: command to patch source after clone
#       [VARS <name> <value> <type> ...]  # optional: set cache variables before FetchContent
#                                         # only applied when fetching, not when importing
#                                         # type required: BOOL STRING PATH FILEPATH INTERNAL
#                                         # e.g. "CPP_LOGGER_LIBDIR_AS_LIB ON BOOL"
#                                         #      "CMAKE_INSTALL_LIBDIR lib PATH"
#       [REQUIRED]                        # optional flag: fatal error if not found or fetched
#       [QUIET]                           # optional flag: suppress status and warning messages
#                                         # cmake configuration continues on failure regardless
#       [EXACT]                           # optional flag: require exact version match
#       [SHALLOW]                         # optional flag: perform a shallow git clone
#       [DIAG]                            # optional flag: enable diagnostic messages
#   )
#
# Output variables (in caller scope):
#   <primary_name>_FOUND
#   <primary_name>_LIBRARIES
#   <primary_name>_INCLUDE_DIRS
#   <primary_name>_POPULATED
#
# Requires CMake >= 3.14 for FetchContent fallback.

include_guard(GLOBAL)

#cmake_minimum_required(VERSION 3.14)

macro(_fetch_package_debug_vars dbg_target_name dbg_libs dbg_includes)
    if (NOT ARG_QUIET AND ARG_DIAG AND ARG_DIAG)
        message(STATUS "[fetch_package] target : '${dbg_target_name}'")
        message(STATUS "[fetch_package] ${pkg_LIBS_var} : '${dbg_libs}'")
        message(STATUS "[fetch_package] ${pkg_INCLUDES_var} : '${dbg_includes}'")
    endif ()
endmacro()

# ------------------------------------------------------------------------------
# Internal helper macro: find include directories for a target when
# INTERFACE_INCLUDE_DIRECTORIES is not set by the package's cmake config.
#
# Some packages set old-style <NAME>_INCLUDE_DIRS variables via find_package
# but do not propagate them to INTERFACE_INCLUDE_DIRECTORIES on the target.
# Additionally, the variable name may not match the primary target name —
# e.g. find_package may set CPP_LOGGER_INCLUDE_DIRS instead of
# cpp-logger_INCLUDE_DIRS. This macro iterates over all candidate package
# name variants (from ARG_NAME) to find whichever _INCLUDE_DIRS variable
# was actually set, then applies it to the target's INTERFACE_INCLUDE_DIRECTORIES
# so consumers can use target_link_libraries() without manually setting includes.
#
# Arguments:
#   _fp_target    — the target to set INTERFACE_INCLUDE_DIRECTORIES on
#   _fp_arg_name  — the name of the list variable containing candidate
#                   package names (pass as string, e.g. ARG_NAME not ${ARG_NAME})
#
# Accesses from caller scope (macro):
#   pkg_INCLUDES_var — the output variable name for include dirs
#
# Sets in caller scope:
#   _includes          — the found include dirs (empty if not found)
#   ${pkg_INCLUDES_var} — propagated to PARENT_SCOPE if found
# ------------------------------------------------------------------------------
macro(_fetch_package_find_includes _fp_target _fp_arg_name)
    set(_includes "")
    foreach(_fp_name IN LISTS ${_fp_arg_name})
        set(_fp_alt_var "${_fp_name}_INCLUDE_DIRS")
        if (NOT "${${_fp_alt_var}}" STREQUAL "" AND
            NOT "${${_fp_alt_var}}" MATCHES "NOTFOUND")
            set(_includes "${${_fp_alt_var}}")
            break()
        endif ()
    endforeach ()
    unset(_fp_alt_var)
    unset(_fp_name)

    if (NOT _includes STREQUAL "")
        set_target_properties(${_fp_target} PROPERTIES
            INTERFACE_INCLUDE_DIRECTORIES "${_includes}"
        )
        set(${pkg_INCLUDES_var} "${_includes}" PARENT_SCOPE)
    endif ()
endmacro()

function(fetch_package)

    # --------------------------------------------------------------------------
    # Parse arguments
    # --------------------------------------------------------------------------
    cmake_parse_arguments(ARG
        "REQUIRED;QUIET;EXACT;SHALLOW;DIAG"  # boolean flags
        "VERSION;GIT;TAG;URL;URL_HASH"       # single-value keywords
        "NAME;SUBMODULES;PATCH_COMMAND;VARS" # multi-value keywords
        ${ARGN}
    )

    if (NOT ARG_NAME)
        message(FATAL_ERROR "[fetch_package] NAME is required")
    endif ()

    if (NOT ARG_GIT AND NOT ARG_URL)
        message(FATAL_ERROR "[fetch_package] either GIT or URL must be specified")
    endif ()

    if (ARG_GIT AND ARG_URL)
        message(FATAL_ERROR "[fetch_package] GIT and URL are mutually exclusive")
    endif ()

    # --------------------------------------------------------------------------
    # Build find_package argument list
    # --------------------------------------------------------------------------
    set(find_args "")
    set(locail_find_args "")
    if (ARG_VERSION)
        list(APPEND find_args ${ARG_VERSION})
        if (ARG_EXACT)
            list(APPEND find_args EXACT)
        endif ()
        set(local_find_args ${find_args})
    endif ()
    if (ARG_REQUIRED)
        list(APPEND find_args REQUIRED)
    endif ()
    if (ARG_QUIET)
        list(APPEND find_args QUIET)
    endif ()

    # --------------------------------------------------------------------------
    # Build FetchContent argument list
    # --------------------------------------------------------------------------
    set(fetch_args "")
    if (ARG_GIT)
        list(APPEND fetch_args GIT_REPOSITORY ${ARG_GIT})
        if (ARG_TAG)
            list(APPEND fetch_args GIT_TAG    ${ARG_TAG})
        endif ()
        if (ARG_SHALLOW)
            list(APPEND fetch_args GIT_SHALLOW TRUE)
        else ()
            list(APPEND fetch_args GIT_SHALLOW FALSE)
        endif ()
        if (DEFINED ARG_SUBMODULES)
            list(APPEND fetch_args GIT_SUBMODULES ${ARG_SUBMODULES})
            if (ARG_SUBMODULES)
                list(APPEND fetch_args GIT_SUBMODULES_RECURSE ON)
            else ()
                list(APPEND fetch_args GIT_SUBMODULES_RECURSE OFF)
            endif ()
        else ()
            list(APPEND fetch_args GIT_SUBMODULES "")
            list(APPEND fetch_args GIT_SUBMODULES_RECURSE OFF)
        endif ()
    elseif (ARG_URL)
        list(APPEND fetch_args URL ${ARG_URL})
        if (ARG_URL_HASH)
            list(APPEND fetch_args URL_HASH ${ARG_URL_HASH})
        endif ()
    endif ()

    if (ARG_PATCH_COMMAND)
        list(APPEND fetch_args PATCH_COMMAND ${ARG_PATCH_COMMAND})
    endif ()

    # Enable correct behavior for GIT_SUBMODULES ""
    if (POLICY CMP0097)
        cmake_policy(SET CMP0097 NEW)
    endif ()

    # --------------------------------------------------------------------------
    # Collect environment hints and populate CMAKE_PREFIX_PATH
    # --------------------------------------------------------------------------
    foreach (pkg_name IN LISTS ARG_NAME)
        set(pkg_dir_hints "")
        foreach (env_var "${pkg_name}_DIR" "${pkg_name}_PATH" "${pkg_name}_ROOT")
            set(_env_val "$ENV{${env_var}}")
            if (NOT _env_val STREQUAL "")
                list(APPEND pkg_dir_hints "${_env_val}")
                if (NOT ARG_QUIET AND ARG_DIAG)
                    message(STATUS "[fetch_package] adding env ${env_var}='${_env_val}'")
                endif ()
            endif ()
        endforeach ()

        foreach (pkg_dir IN LISTS pkg_dir_hints)
            list(APPEND CMAKE_PREFIX_PATH "${pkg_dir}")
            list(APPEND CMAKE_PREFIX_PATH "${pkg_dir}/lib")
            list(APPEND CMAKE_PREFIX_PATH "${pkg_dir}/lib64")
            list(APPEND CMAKE_PREFIX_PATH "${pkg_dir}/lib/cmake/${pkg_name}")
            list(APPEND CMAKE_PREFIX_PATH "${pkg_dir}/lib64/cmake/${pkg_name}")
        endforeach ()
    endforeach ()
    # Propagate hints to caller so subsequent find_package calls also benefit
    #set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH}" PARENT_SCOPE)

    # --------------------------------------------------------------------------
    # Attempt find_package first
    # --------------------------------------------------------------------------
    list(GET ARG_NAME 0 primary_name)

    # Derive output variable names
    set(pkg_FOUND_var     "${primary_name}_FOUND")
    set(pkg_LIBS_var      "${primary_name}_LIBRARIES")
    set(pkg_INCLUDES_var  "${primary_name}_INCLUDE_DIRS")

    if (NOT ARG_QUIET AND ARG_DIAG)
        message(STATUS "[fetch_package] attempt find_package(${primary_name}; ${find_args} NAMES ${ARG_NAME}; QUIET)")
    endif ()

    find_package(${primary_name} ${local_find_args} NAMES ${ARG_NAME} QUIET)

    if (${pkg_FOUND_var})
        if (NOT ARG_QUIET AND ARG_DIAG)
            message(STATUS "[fetch_package] find_package found ${primary_name}")
        endif ()

        set(_lib_dir "")
        foreach(_target ${scoped_target} ${primary_name})
            if (TARGET ${_target} AND _lib_dir STREQUAL "")
                set(_loc "")

                # Promote target to global visibility so parent scopes can use it
                set_target_properties(${_target} PROPERTIES IMPORTED_GLOBAL TRUE)

                # Try consumer's build type first
                if (CMAKE_BUILD_TYPE)
                    string(TOUPPER "${CMAKE_BUILD_TYPE}" _build_type_upper)
                    get_target_property(_loc ${_target} IMPORTED_LOCATION_${_build_type_upper})
                    if (_loc STREQUAL "" OR _loc MATCHES "NOTFOUND")
                        set(_loc "")
                    endif ()
                endif ()
    
                # Fallback to generic IMPORTED_LOCATION
                if (_loc STREQUAL "")
                    get_target_property(_loc ${_target} IMPORTED_LOCATION)
                    if (_loc STREQUAL "" OR _loc MATCHES "NOTFOUND")
                        set(_loc "")
                    endif ()
                endif ()
    
                # Fall back to other configs
                if (_loc STREQUAL "")
                    foreach(_config RELEASE DEBUG RELWITHDEBINFO MINSIZEREL NOCONFIG)
                        if (NOT _config STREQUAL "${_build_type_upper}")
                            get_target_property(_loc_cfg ${_target} IMPORTED_LOCATION_${_config})
                            if (NOT _loc_cfg STREQUAL "" AND NOT _loc_cfg MATCHES "NOTFOUND")
                                set(_loc "${_loc_cfg}")
                                break()
                            endif ()
                        endif ()
                    endforeach ()
                endif ()
    
                if (NOT _loc STREQUAL "")
                    get_filename_component(_lib_dir "${_loc}" DIRECTORY)
                    #message(STATUS "[fetch_package] setting INTERFACE_LINK_DIRECTORIES: '${_lib_dir}' on '${_target}'")
                    set_target_properties(${_target} PROPERTIES
                        INTERFACE_LINK_DIRECTORIES "${_lib_dir}"
                    )
                    #get_target_property(_check ${_target} INTERFACE_LINK_DIRECTORIES)
                    #message(STATUS "[fetch_package] ${_target} INTERFACE_LINK_DIRECTORIES: '${_check}'")
                endif ()
            endif ()
        endforeach ()
    else ()
        # ----------------------------------------------------------------------
        # Fall back to FetchContent if not found locally
        # ----------------------------------------------------------------------
        if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.14")
            if (NOT ARG_QUIET AND ARG_DIAG)
                message(STATUS "[fetch_package] ${primary_name} not found locally, fetching...")
            endif ()

            include(FetchContent)
            FetchContent_Declare(${primary_name} ${fetch_args})

            # Force set cmake variable caches
            if (ARG_VARS)
                foreach(_opt IN LISTS ARG_VARS)
                    unset(CMAKE_MATCH_1)
                    unset(CMAKE_MATCH_2)
                    unset(CMAKE_MATCH_3)
                
                    string(REGEX MATCH "^([^ ]+) (.+) (BOOL|STRING|PATH|FILEPATH|INTERNAL)$"
                           _match "${_opt}")
                    if (CMAKE_MATCH_1 AND CMAKE_MATCH_2 AND CMAKE_MATCH_3)
                        set(${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE ${CMAKE_MATCH_3} "" FORCE)
                    else ()
                        unset(CMAKE_MATCH_1)
                        unset(CMAKE_MATCH_2)
                        string(REGEX MATCH "^([^ ]+) (.+)$" _match "${_opt}")
                        if (CMAKE_MATCH_1 AND CMAKE_MATCH_2)
                            set(${CMAKE_MATCH_1} "${CMAKE_MATCH_2}" CACHE STRING "" FORCE)
                        else ()
                            if (NOT ARG_QUIET AND ARG_DIAG)
                                message(WARNING "[fetch_package] invalid VARS entry: '${_opt}'")
                            endif ()
                        endif ()
                    endif ()
                endforeach ()
            endif ()

            # Suppress deprecation warnings from the fetched project's older CMake policies
            set(CMAKE_WARN_DEPRECATED OFF CACHE BOOL "" FORCE)
            FetchContent_MakeAvailable(${primary_name})
            set(CMAKE_WARN_DEPRECATED ON  CACHE BOOL "" FORCE)

            # Suppress warnings from fetched project's sources
            if (TARGET ${primary_name})
                target_compile_options(${primary_name} PRIVATE -w)
            endif ()

            # Propagate FetchContent population status to caller scope
            FetchContent_GetProperties(${primary_name})
            set(${primary_name}_POPULATED "${${primary_name}_POPULATED}" PARENT_SCOPE)
        else ()
            # CMake too old for FetchContent
            if (ARG_REQUIRED)
                message(FATAL_ERROR
                    "[fetch_package] ${primary_name} not found and FetchContent "
                    "requires CMake >= 3.14 (current: ${CMAKE_VERSION})"
                )
            elseif (NOT ARG_QUIET AND ARG_DIAG)
                message(WARNING
                    "[fetch_package] ${primary_name} not found and FetchContent "
                    "requires CMake >= 3.14 (current: ${CMAKE_VERSION})"
                )
            endif ()
            return()
        endif ()
    endif ()

    # --------------------------------------------------------------------------
    # Resolve target and populate output variables
    # --------------------------------------------------------------------------
    set(scoped_target "${primary_name}::${primary_name}")

    if (TARGET ${scoped_target})
        # Modern imported target with scoped name (preferred)
        get_target_property(_includes ${scoped_target} INTERFACE_INCLUDE_DIRECTORIES)
        _fetch_package_debug_vars(${scoped_target} ${scoped_target} "${_includes}")

        if (NOT _includes STREQUAL "" AND NOT _includes MATCHES "NOTFOUND")
            if ("${${pkg_INCLUDES_var}}" STREQUAL "" OR
                "${${pkg_INCLUDES_var}}" MATCHES "NOTFOUND")
                set(${pkg_INCLUDES_var} "${_includes}"  PARENT_SCOPE)
            else ()
                # Preserve the value already set by find_package
                set(${pkg_INCLUDES_var} "${${pkg_INCLUDES_var}}" PARENT_SCOPE)
            endif ()
        else ()
            _fetch_package_find_includes(${scoped_target} ARG_NAME)
        endif ()
        set(${pkg_LIBS_var}          "${scoped_target}" PARENT_SCOPE)
        set(${pkg_FOUND_var}         TRUE               PARENT_SCOPE)

    elseif (TARGET ${primary_name})
        # Unscoped imported target
        get_target_property(_includes ${primary_name} INTERFACE_INCLUDE_DIRECTORIES)
        _fetch_package_debug_vars(${primary_name} ${primary_name} "${_includes}")

        if (NOT _includes STREQUAL "" AND NOT _includes MATCHES "NOTFOUND")
            if ("${${pkg_INCLUDES_var}}" STREQUAL "" OR
                "${${pkg_INCLUDES_var}}" MATCHES "NOTFOUND")
                set(${pkg_INCLUDES_var} "${_includes}"  PARENT_SCOPE)
            else ()
                set(${pkg_INCLUDES_var} "${${pkg_INCLUDES_var}}" PARENT_SCOPE)
            endif ()
        else ()
            _fetch_package_find_includes(${primary_name} ARG_NAME)
        endif ()
        set(${pkg_LIBS_var}          "${primary_name}"  PARENT_SCOPE)
        set(${pkg_FOUND_var}         TRUE               PARENT_SCOPE)

    elseif (NOT "${${pkg_LIBS_var}}" STREQUAL "" AND
            NOT "${${pkg_LIBS_var}}" MATCHES "NOTFOUND")
        # Old-style variables set by find_package — wrap in an imported target
        set(_orig_libs     "${${pkg_LIBS_var}}")
        set(_orig_includes "${${pkg_INCLUDES_var}}")
    
        add_library(${scoped_target} UNKNOWN IMPORTED)
    
        if (NOT _orig_includes STREQUAL "" AND NOT _orig_includes MATCHES "NOTFOUND")
            set_target_properties(${scoped_target} PROPERTIES
                IMPORTED_LOCATION             "${_orig_libs}"
                INTERFACE_INCLUDE_DIRECTORIES "${_orig_includes}"
            )
            set(${pkg_INCLUDES_var} "${_orig_includes}" PARENT_SCOPE)
        else ()
            set_target_properties(${scoped_target} PROPERTIES
                IMPORTED_LOCATION "${_orig_libs}"
            )
        endif ()

        _fetch_package_debug_vars("${scoped_target}" "${_orig_libs}" "${_orig_includes}")
        set(${pkg_LIBS_var}  "${scoped_target}" PARENT_SCOPE)
        set(${pkg_FOUND_var} TRUE               PARENT_SCOPE)

    else ()
        # Could not find or create any target
        set(${pkg_FOUND_var} FALSE PARENT_SCOPE)
        if (ARG_REQUIRED)
            message(FATAL_ERROR
                "[fetch_package] could not find or create a target for ${primary_name}"
            )
        elseif (NOT ARG_QUIET)
            message(WARNING
                "[fetch_package] could not find or create a target for ${primary_name}"
            )
        endif ()
        return()
    endif ()
endfunction()
