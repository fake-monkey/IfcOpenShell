from conan import ConanFile
from conan.tools import cmake
from conan.tools.cmake import CMakeToolchain

class IFCOpenShellRecipe(ConanFile):
    name = "ifcopenshell"
    version = "0.8.2"

    settings = "os", "arch"

    def requirements(self):
        self.requires("opencascade/7.8.0.patch.1@third_party/stable")
        self.requires("boost/1.85.0@third_party/stable")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = None  # prevent CMakeUserPresets.json from being generated
        tc.generate()
