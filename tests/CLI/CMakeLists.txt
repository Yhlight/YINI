# Add a test that runs the shell script
add_test(
    NAME CLITest
    COMMAND ${CMAKE_COMMAND} -E echo "Running CLI test..." &&
            ${CMAKE_SOURCE_DIR}/tests/CLI/test_cli.sh $<TARGET_FILE:yini_cli>
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

# Ensure the CLI executable is built before the test runs
add_dependencies(test_ymeta yini_cli)