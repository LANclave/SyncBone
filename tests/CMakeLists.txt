cmake_minimum_required(VERSION 3.20)

# Simple functional test: copy sample.txt to copy_sample_ctest.txt

# Verify output file content

add_test(NAME sync_whole_tree
    COMMAND $<TARGET_FILE:syncbone> ${CMAKE_SOURCE_DIR}/test_data ${CMAKE_BINARY_DIR}/synced_data)
set_tests_properties(sync_whole_tree PROPERTIES WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")

# Verify several representative files exist after sync
add_test(NAME verify_synced_file1
    COMMAND ${CMAKE_COMMAND} -E compare_files
            ${CMAKE_SOURCE_DIR}/test_data/dirA/file1.txt
            ${CMAKE_BINARY_DIR}/synced_data/dirA/file1.txt)
add_test(NAME verify_synced_file2
    COMMAND ${CMAKE_COMMAND} -E compare_files
            ${CMAKE_SOURCE_DIR}/test_data/dirA/dirB/file2.txt
            ${CMAKE_BINARY_DIR}/synced_data/dirA/dirB/file2.txt)
add_test(NAME verify_unicode_file
    COMMAND ${CMAKE_COMMAND} -E compare_files
            ${CMAKE_SOURCE_DIR}/test_data/unicode_файл.txt
            ${CMAKE_BINARY_DIR}/synced_data/unicode_файл.txt)

set_tests_properties(verify_synced_file1 verify_synced_file2 verify_unicode_file PROPERTIES DEPENDS sync_whole_tree WORKING_DIRECTORY "${CMAKE_BINARY_DIR}")
