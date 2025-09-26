cmake_minimum_required(VERSION 3.20)

# Simple functional test: copy sample.txt to copy_sample_ctest.txt
add_test(NAME copy_sample_file
    COMMAND $<TARGET_FILE:syncbone> ${CMAKE_SOURCE_DIR}/test_data/sample.txt ${CMAKE_BINARY_DIR}/copy_sample_ctest.txt)
set_tests_properties(copy_sample_file PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

# Verify output file content
add_test(NAME verify_sample_file_content
    COMMAND ${CMAKE_COMMAND} -E compare_files ${CMAKE_SOURCE_DIR}/test_data/sample.txt ${CMAKE_BINARY_DIR}/copy_sample_ctest.txt)
set_tests_properties(verify_sample_file_content PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

set_tests_properties(verify_sample_file_content PROPERTIES DEPENDS copy_sample_file)
