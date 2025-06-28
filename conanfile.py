from conan import ConanFile
from conan.tools import cmake
from conan.tools.cmake import CMakeToolchain
from os import path
from pathlib import Path
import shutil

class IFCOpenShellRecipe(ConanFile):
    name = "ifcopenshell"
    version = "0.8.2"
    user = "third_party"
    channel = "stable"

    settings = "os", "arch"

    def requirements(self):
        self.requires("opencascade/7.8.0.patch.1@third_party/stable")
        self.requires("boost/1.85.0@third_party/stable")
        self.requires("eigen/3.4.0@third_party/stable")

    def generate(self):
        tc = CMakeToolchain(self)
        tc.user_presets_path = None  # prevent CMakeUserPresets.json from being generated
        for k, v in self.dependencies._data.items():
            pkg = str(v)[:str(v).find('/')]
            pkg_name = str(self.dependencies[pkg].description)
            tc.variables[pkg_name + "_DIR"] = Path(
                path.join(self.dependencies[pkg].package_folder, "cmake")).as_posix()
        tc.generate()
        
    def package(self):
        src_dir = Path(self.source_folder) / "build" / "package"
        target_dir = self.package_folder
        if path.exists(target_dir):
            shutil.rmtree(target_dir)
        shutil.copytree(src_dir, target_dir)
        print(f"{src_dir} -> {target_dir}")

    def package_info(self):
        self.cpp_info.builddirs = ["cmake"]
        self.cpp_info.set_property("cmake_find_mode", "none")
