project(exole)
cmake_minimum_required(VERSION 3.0)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)
include(FindEditline)
find_package(Editline)

add_definitions(-std=c++14 -pedantic -Wall -Werror -D_GLIBCXX_USE_CXX11_ABI=0)

add_library(exole SHARED
    application.cpp
    console.cpp
    token_parser.cpp
    wcs_util.cpp
    file_name_completer.cpp
    command_manager.cpp
    help_command.cpp
    pagination.cpp
    detail/arguments.cpp
    )
target_link_libraries(exole ${EDITLINE_LIBRARIES})
add_executable(example example.cpp)
target_link_libraries(example exole)
add_executable(example_help example_help.cpp)
target_link_libraries(example_help exole)
install(FILES
    application.h
    console.h
    command.h
    command_manager.h
    command_context.h
    help_command.h
    wcs_util.h
    completion.h
    file_name_completer.h
    token_parser.h
    pagination.h
    DESTINATION include/exole
    )
install(TARGETS exole LIBRARY DESTINATION lib)
