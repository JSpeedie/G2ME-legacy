cmake_minimum_required(VERSION 3.16)
# Use C11
set(CMAKE_C_STANDARD 11)

project(G2ME)

# Tell CMake to prefer the `-pthread` flag over `-lpthread`
set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

#------------------------------------------------------------------------------
#   Executables
#------------------------------------------------------------------------------
add_executable(G2ME G2ME.c glicko2.c entry.c p_files.c pr.c opp_files.c
	tournament_files.c printing.c fileops.c sorting.c player_dir.c data_dir.c)
target_link_libraries(G2ME PRIVATE m)
target_link_libraries(G2ME PRIVATE Threads::Threads)

add_executable(G2ME-client client.c clientserverutil.c)

add_executable(G2ME-server server.c clientserverutil.c)

#------------------------------------------------------------------------------
#   Building Tests
#------------------------------------------------------------------------------
add_custom_target(build_tests
	DEPENDS player_tests entry_tests p_file_tests output_tests
	VERBATIM
)

# All the executables below specify `EXCLUDE_FROM_ALL` so that they are not
# built by default

add_executable(player_tests EXCLUDE_FROM_ALL
	tests/player_tests.c glicko2.c)
target_link_libraries(player_tests PRIVATE m)
target_link_libraries(player_tests PRIVATE criterion)

add_executable(entry_tests EXCLUDE_FROM_ALL
	tests/entry_tests.c glicko2.c entry.c)
target_link_libraries(entry_tests PRIVATE m)
target_link_libraries(entry_tests PRIVATE criterion)

add_executable(p_file_tests EXCLUDE_FROM_ALL
	tests/p_file_tests.c p_files.c opp_files.c tournament_files.c entry.c
	player_dir.c data_dir.c glicko2.c fileops.c)
target_link_libraries(p_file_tests PRIVATE m)
target_link_libraries(p_file_tests PRIVATE criterion)

add_executable(output_tests EXCLUDE_FROM_ALL
	tests/output_tests.c G2ME.c glicko2.c entry.c p_files.c pr.c opp_files.c
	tournament_files.c printing.c fileops.c sorting.c player_dir.c data_dir.c)
target_link_libraries(output_tests PRIVATE m)
target_link_libraries(output_tests PRIVATE criterion)

set_target_properties(
	player_tests entry_tests p_file_tests output_tests
	PROPERTIES
		RUNTIME_OUTPUT_DIRECTORY tests/bin
)

#------------------------------------------------------------------------------
#   Running Tests
#------------------------------------------------------------------------------

# Copy the compiled test executables from the build directory to `tests`
add_custom_target(copy_unit_test_binaries
	COMMAND ${CMAKE_COMMAND} -E copy_directory
		"${CMAKE_BINARY_DIR}/tests/bin"
		"${CMAKE_CURRENT_SOURCE_DIR}/tests/bin"
	DEPENDS player_tests entry_tests p_file_tests output_tests
	VERBATIM
)

add_custom_target(run_unit_tests
	COMMAND bash -c "for test in $(ls -1 bin); do echo; echo \"Running ./bin/\$test\"; ./bin/\$test --verbose=1; done"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests"
	DEPENDS copy_unit_test_binaries
	USES_TERMINAL
	VERBATIM
)

add_custom_target(copy_integration_test_binaries
	COMMAND ${CMAKE_COMMAND} -E copy
		"${CMAKE_BINARY_DIR}/G2ME"
		"${CMAKE_CURRENT_SOURCE_DIR}/tests/bin/G2ME"
	DEPENDS G2ME
	VERBATIM
)

add_custom_target(run_integration_tests
	COMMAND bash -c "python3 runtestcases.py"
	WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/tests"
	DEPENDS copy_integration_test_binaries
	USES_TERMINAL
	VERBATIM
)

add_custom_target(run_tests
	DEPENDS run_unit_tests run_integration_tests
)

#------------------------------------------------------------------------------
#   Installation
#------------------------------------------------------------------------------

install(TARGETS G2ME G2ME-server G2ME-client
    RUNTIME DESTINATION bin
)
