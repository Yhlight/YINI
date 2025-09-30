from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

class YiniConan(ConanFile):
    name = "yini"
    version = "1.0.0"
    license = "MIT"
    author = "Yhlight"
    url = "https://github.com/your-repo/yini" # Replace with actual URL
    description = "A powerful and feature-rich configuration file format library."
    topics = ("configuration", "ini", "parser", "cpp")
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}

    exports_sources = "CMakeLists.txt", "src/*", "include/*", "LICENSE"

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.generate()

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()

    def package_info(self):
        self.cpp_info.libs = ["yini"]
        self.cpp_info.includedirs = ["include"]