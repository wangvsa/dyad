if (CMAKE_INSTALL_LIBDIR STREQUAL "lib" AND CMAKE_SIZEOF_VOID_P EQUAL 8)
    install(CODE "
        if (NOT EXISTS \"\${CMAKE_INSTALL_PREFIX}/lib\")
            message(STATUS \"Skipping symlink: \${CMAKE_INSTALL_PREFIX}/lib does not exist yet\")
        else()
            if (NOT EXISTS \"\${CMAKE_INSTALL_PREFIX}/lib64\")
                execute_process(
                    COMMAND ${CMAKE_COMMAND} -E create_symlink
                        \${CMAKE_INSTALL_PREFIX}/lib
                        \${CMAKE_INSTALL_PREFIX}/lib64
                    RESULT_VARIABLE _symlink_result
                )
                if (_symlink_result)
                    message(WARNING \"Failed to create symlink lib64 -> lib\")
                else()
                    message(STATUS \"Created symlink: \${CMAKE_INSTALL_PREFIX}/lib64 -> \${CMAKE_INSTALL_PREFIX}/lib\")
                endif()
            elseif (IS_SYMLINK \"\${CMAKE_INSTALL_PREFIX}/lib64\")
                message(STATUS \"Skipping symlink: \${CMAKE_INSTALL_PREFIX}/lib64 already exists as symlink\")
            else()
                message(STATUS \"Skipping symlink: \${CMAKE_INSTALL_PREFIX}/lib64 exists as real directory\")
            endif()
        endif()
    ")
endif()
