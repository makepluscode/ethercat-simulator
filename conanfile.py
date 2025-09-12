from conan import ConanFile
import os


class EtherCATSimulatorRecipe(ConanFile):
    name = "ethercat-simulator"
    version = "0.1.0"
    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    package_type = "application"
    requires = ("gtest/1.14.0",)
    options = {"with_kickcat": [True, False]}
    default_options = {"with_kickcat": False}

    def layout(self):
        # Rely on default src/include at repo root; build folder provided by -of build
        pass

    def requirements(self):
        if bool(self.options.get_safe("with_kickcat")):
            ref = os.environ.get("KICKCAT_REF")
            if ref:
                self.requires(ref)
            else:
                self.output.warning(
                    "with_kickcat=True but KICKCAT_REF not set; skipping KickCAT requirement"
                )
