# This is a basic portfile for the YINI library.
# It assumes it's being built from a local source tree.

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO your-repo/YINI # Placeholder for the actual repo
    REF "v${VERSION}"   # Use the version from vcpkg.json
    SHA512 0 # Placeholder, would be calculated for a real release
    HEAD_REF main
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DYINI_BUILD_CLI=OFF # Don't build the CLI tool for the library package
        -DBUILD_TESTING=OFF # Don't build tests
)

vcpkg_cmake_install()

vcpkg_copy_pdbs()

# Install the license file
file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

# Handle CMake config files
vcpkg_cmake_config_fixup(PACKAGE_NAME Yini)