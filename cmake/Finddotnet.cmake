# - Find dotnet
#
# This module finds the dotnet CLI executable.
#
# This module will define the following variables:
#  dotnet_FOUND        - True if the dotnet executable was found.
#  dotnet_EXECUTABLE   - The path to the dotnet executable.

find_program(dotnet_EXECUTABLE dotnet)

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(dotnet DEFAULT_MSG dotnet_EXECUTABLE)

mark_as_advanced(dotnet_EXECUTABLE)