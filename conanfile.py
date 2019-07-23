from conans import ConanFile, CMake, tools


class KnotificationsConan(ConanFile):
    name = "knotifications"
    version = "5.61.0"
    license = "GPLv2"
    url = "https://api.kde.org/frameworks/knotifications/html/index.html"
    description = "Desktop notifications"
    settings = "os", "compiler", "build_type", "arch"
    default_options = (
        "qt:qtx11extras = True",
        "qt:qtdeclarative = True",
        "qt:with_pq=False"
    )

    requires = (
        "extra-cmake-modules/[>=5.60.0]@kde/testing", # CMakeLists.txt requires 5.49.0
        "qt/[>=5.11.3]@bincrafters/stable", # +x11 extras
        "kwindowsystem/[>=5.60.0]@kde/testing",
        "kconfig/[>=5.60.0]@kde/testing",
        "kcodecs/[>=5.60.0]@kde/testing",
        "kcoreaddons/[>=5.60.0]@kde/testing",
        #"canberra/X.Y.Z@kde/testing" OPTIONAL
        "phonon4qt5/[>=4.6.60]@kde/testing",
    )

    generators = "cmake"

    scm = {
        "type": "git",
        "url": "auto",
        "revision": "auto"
     }

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()
        cmake.install()

    def package_info(self):
        self.cpp_info.resdirs = ["share"]
