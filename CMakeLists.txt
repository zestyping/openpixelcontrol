cmake_minimum_required(VERSION 2.8)
project(openpixelprotocol)

# We use a newer version of FindGLUT that findws windows DLLs
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if (WIN32)
    set(GLUT_DIR "" CACHE PATH "Path to the directory where the GLUT binaries can be found, glut32.lib, glut32.dll, glut.h" )
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(LINUX 1)
endif()

option(WITH_DUMMY_CLIENT "Compile the Dummy client" ON)
option(WITH_DUMMY_SERVER "Compile the Dummy server" ON)
option(WITH_GL_SERVER "GL Server (Requires GLUT)" ON)

if (LINUX)
    option(WITH_TCL_SERVER "Compile TCL server" ON)
    option(WITH_APA102_SERVER "Compile APA102 server" ON)
    option(WITH_WS2801_SERVER "Compile WS2801 server" ON)
    option(WITH_LPD8806_SERVER "Compile LPD8806 server" ON)
endif()

# Set some nicer output dirs.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

if (MSVC)
    add_definitions("/W3 /D_CRT_SECURE_NO_WARNINGS /wd4005 /wd4996 /nologo")
    set(CMAKE_EXE_LINKER_FLAGS "/SAFESEH:NO")
endif()

if (APPLE)
    set(CMAKE_C_FLAGS "-Wdeprecated-declarations")
endif()

if (WITH_DUMMY_CLIENT)
    set(DUMMY_CLIENT_SRCS
        ${PROJECT_SOURCE_DIR}/src/dummy_client.c
        ${PROJECT_SOURCE_DIR}/src/opc_client.c)
    
    set(DUMMY_CLIENT_HDRS
        ${PROJECT_SOURCE_DIR}/src/types.h
        ${PROJECT_SOURCE_DIR}/src/opc.h
        ${PROJECT_SOURCE_DIR}/src/compat.h)
    
    source_group("Headers" FILES ${DUMMY_CLIENT_HDRS})
    source_group("Sources" FILES ${DUMMY_CLIENT_SRCS})

    add_executable(dummy_client ${DUMMY_CLIENT_SRCS} ${DUMMY_CLIENT_HDRS})

    if (WIN32)
        target_link_libraries(dummy_client ws2_32.lib)
    endif()
endif()

if (WITH_DUMMY_SERVER)
    set(DUMMY_SERVER_SRCS
        ${PROJECT_SOURCE_DIR}/src/dummy_server.c
        ${PROJECT_SOURCE_DIR}/src/opc_server.c)
    
    set(DUMMY_SERVER_HDRS
        ${PROJECT_SOURCE_DIR}/src/types.h
        ${PROJECT_SOURCE_DIR}/src/opc.h)

    source_group("Headers" FILES ${DUMMY_SERVER_HDRS})
    source_group("Sources" FILES ${DUMMY_SERVER_SRCS})

    add_executable(dummy_server ${DUMMY_SERVER_SRCS} ${DUMMY_SERVER_HDRS})

    if (WIN32)
        target_link_libraries(dummy_server ws2_32.lib)
    endif()
endif()

if (WITH_GL_SERVER)
    set(GL_SERVER_SRCS
        ${PROJECT_SOURCE_DIR}/src/gl_server.c
        ${PROJECT_SOURCE_DIR}/src/opc_server.c
        ${PROJECT_SOURCE_DIR}/src/cJSON.c)

    set(GL_SERVER_HDRS
        ${PROJECT_SOURCE_DIR}/src/cJSON.h
        ${PROJECT_SOURCE_DIR}/src/types.h
        ${PROJECT_SOURCE_DIR}/src/opc.h
        ${PROJECT_SOURCE_DIR}/src/win32/getopt.h
        ${PROJECT_SOURCE_DIR}/src/compat.h)

    source_group("Headers" FILES ${GL_SERVER_HDRS})
    source_group("Sources" FILES ${GL_SERVER_SRCS})

    if (WIN32 AND GLUT_DIR)
        set(GLUT_FOUND ON)
        set(GLUT_INCLUDE_DIRS "${GLUT_DIR}")
        set(GLUT_LIBRARIES "${GLUT_DIR}/glut32.lib")
        set(GLUT_RUNTIME_LIBRARY_DIRS "${GLUT_DIR}")
        set(GLUT_RUNTIME_LIBRARY "${GLUT_DIR}/glut32.dll")
    else()
        find_package(GLUT)

        if (NOT GLUT_FOUND)
            if (WIN32)
                message(FATAL_ERROR "Set cmake -DGLUT_DIR=<path>. Run 'cmake -LH .' for details")
            else()
                message(FATAL_ERROR "GLUT is required, please install it")
            endif()
        endif()
    endif()

    add_executable(glserver ${GL_SERVER_SRCS} ${GL_SERVER_HDRS})
    target_link_libraries(glserver ${GLUT_LIBRARIES})
    message("GLUT Include dir: ${GLUT_INCLUDE_DIRS}")
    include_directories("${GLUT_INCLUDE_DIRS}")

    if (WIN32)
        target_link_libraries(glserver ws2_32.lib)

        # Copy GLUT DLL to binary dir.
        add_custom_command(TARGET glserver POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                ${GLUT_RUNTIME_LIBRARY}
                $<TARGET_FILE_DIR:glserver>)
    endif()

    if (LINUX)
        target_link_libraries(glserver m)
    endif()
endif()

if (LINUX)
    set(SERVER_LIST "")

    set(SERVER_COMMON_SRCS
        "${PROJECT_SOURCE_DIR}/src/opc_server.c"
        "${PROJECT_SOURCE_DIR}/src/spi.c"
        "${PROJECT_SOURCE_DIR}/src/cli.c")

    set(SERVER_COMMON_HDRS
        "${PROJECT_SOURCE_DIR}/src/spi.h"
        "${PROJECT_SOURCE_DIR}/src/opc.h"
        "${PROJECT_SOURCE_DIR}/src/types.h"
        "${PROJECT_SOURCE_DIR}/src/cli.h"
        )

    source_group("Headers" FILES ${SERVER_COMMON_HDRS})

    if (WITH_TCL_SERVER)
        list(APPEND SERVER_LIST "tcl_server")
    endif()

    if (WITH_APA102_SERVER)
        list(APPEND SERVER_LIST "apa102_server")
    endif()

    if (WITH_WS2801_SERVER)
        list(APPEND SERVER_LIST "ws2801_server")
    endif()

    if (WITH_LPD8806_SERVER)
        list(APPEND SERVER_LIST "lpd8806_server")
    endif()

    foreach (SERVER ${SERVER_LIST})
        set(SRCS
            "${PROJECT_SOURCE_DIR}/src/${SERVER}.c"
            ${SERVER_COMMON_SRCS})

        source_group("Sources" FILES ${SRCS})

        add_executable(${SERVER} ${SRCS} ${HDRS})
        target_link_libraries(${SERVER} m)
    endforeach()
endif()
