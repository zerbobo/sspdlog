cmake_minimum_required(VERSION 3.2)

project(sspdlog)

###### example
if(UNIX)
    add_compile_options(-std=c++11)
endif()

include_directories(${sspdlog_SOURCE_DIR}/spdlog/include)
include_directories(${sspdlog_SOURCE_DIR}/include)

add_executable(example1 example/example1.cpp)
add_executable(example2 example/example2.cpp)
add_executable(example3 example/example3.cpp)
add_executable(example4 example/example4.cpp)
if(UNIX)
    target_link_libraries(example1 pthread)
    target_link_libraries(example2 pthread)
    target_link_libraries(example3 pthread)
    target_link_libraries(example4 pthread)
endif()
add_custom_command(TARGET example1
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${sspdlog_SOURCE_DIR}/example/example1_config.txt $<TARGET_FILE_DIR:example1>/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${sspdlog_SOURCE_DIR}/example/example1_config.txt ${CMAKE_CURRENT_BINARY_DIR}/
    )
add_custom_command(TARGET example2
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${sspdlog_SOURCE_DIR}/example/example2_config.txt $<TARGET_FILE_DIR:example2>/
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${sspdlog_SOURCE_DIR}/example/example2_config.txt ${CMAKE_CURRENT_BINARY_DIR}/
    )


###### tests
enable_testing()
find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

add_executable(sspdlog_basic_test tests/sspdlog_basic_test.cpp)
if(UNIX)
    target_link_libraries(sspdlog_basic_test ${GTEST_BOTH_LIBRARIES} pthread)
else()
    target_link_libraries(sspdlog_basic_test ${GTEST_BOTH_LIBRARIES})
endif()
add_test(test_sspdlog_all sspdlog_basic_test)


##### install
install(DIRECTORY ${sspdlog_SOURCE_DIR}/spdlog/include/spdlog DESTINATION include)
install(DIRECTORY ${sspdlog_SOURCE_DIR}/include/sspdlog DESTINATION include)
