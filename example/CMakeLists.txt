include_directories(..)

add_executable(hex_view hex_view.cpp)
target_link_libraries(hex_view exole)

add_executable(example_help help.cpp)
target_link_libraries(example_help exole)

add_executable(example_batch batch.cpp)
target_link_libraries(example_batch exole)
