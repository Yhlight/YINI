from conan import ConanFile
from conan.tools.cmake import CMake, CMakeToolchain, cmake_layout

class YiniConan(ConanFile):
    name = "yini"
    version = "0.1.0"
    license = "MIT"
    author = "Yini-Author"
    url = "https://github.com/your-repo/YINI"
    description = "A modern INI-style configuration file parser"
    settings = "os", "compiler", "build_type", "arch"
    options = {"shared": [True, False], "fPIC": [True, False]}
    default_options = {"shared": False, "fPIC": True}
    exports_sources = "src/*", "include/*", "CMakeLists.txt"

    def requirements(self):
        self.requires("nlohmann_json/3.11.2")

    def config_options(self):
        if self.settings.os == "Windows":
            self.options.rm_safe("fPIC")

    def layout(self):
        cmake_layout(self)

    def generate(self):
        tc = CMakeToolchain(self)
        tc.variables["YINI_BUILD_TESTS"] = "OFF"
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
        self.cpp_info.bindirs = ["bin"]