# flow5
flow5 is a potential flow solver with built-in pre- and post- processing functionalities. Its purpose is to make the preliminary designs of wings, planes, hydrofoils and sails reliable, fast and user-friendly.

It is version 7 of the legacy project xflr5.


# Remaining developments 
- Fix bugs and implement minor improvements depending on user requests
- ~~Either complete or remove currently inoperative features, e.g. plane optimization, advanced editors~~
- Consider adding force location to extra-drag input
- Continue to clean the code and to improve the API 
- Clean the documentation

# API 

## Implementation
Please note that the API is still in an experimental state and subject to change. 

The intent is to stabilize both the source code and the API by the end of 2026. 

Even though the main part of the intended refactoring has been completed with v7.57, deprecation or even discontinuation of interfaces are still likely until the end of the year.

## Examples
- XFoilRun: Construction of a NACA foil and calculation of a type 1 polar using XFoil.
- XFoilBatchRun: Construction of a NACA foil and multi-threaded calculation of a 2d polar mesh.
- PlaneRun1: Construction of an xfl-type plane with no fuselage and calculation of a type 2 polar with viscous drag evaluated on the fly. Shows how to use custom units.
- PlaneRun2: Construction of an xfl-type plane with a NURBS type fuselage, construction of a conforming mesh, and calculation of a type 2 polar.
- PlaneRun3: Import of an stl-type plane from a mesh file, configuration of the trailing edge and calculation of a type 1 polar.
- PlaneRun4: Import of a plane and a polar from xml files, construction of a conforming mesh, and calculation of a type 1 polar.
- PlaneRun5: Import of an STL-type fuselage into an xfl-type plane.
- PlaneRun6: Import of a fuselage from a STEP file into an xfl-type plane.

## Get Started

### Windows

1. Download the latest release from the **https://github.com/techwinder/flow5/releases** page.
2. Extract the `flow5_v7.xx_win64.zip`.
3. Run `flow5.exe`. 
4. If prompted by Windows SmartScreen, click **More info** → **Run anyway**.
5. You're ready to create your first project.

### Linux

#### Option 1: Build from Source (Recommended)

## Building from Source (Linux)

Flow5 uses **Qt 6** and is developed primarily with **Qt Creator**. The recommended development environment is Qt Creator with the Qt 6 development libraries installed.

### Requirements

Before building Flow5, install:

* Git
* A C++17 compatible compiler (GCC or Clang)
* CMake
* Qt 6 development libraries
* Qt Creator (recommended)
* OpenBLAS and LAPACK (or Intel MKL)
* OpenCascade Community Edition (OCCT) development libraries
* Gmsh SDK / development libraries

> Package names vary between Linux distributions.

### Install Dependencies

#### Arch Linux

```bash
sudo pacman -S --needed \
    git base-devel cmake ninja \
    qt6-base qt6-tools qtcreator \
    openblas lapack \
    opencascade gmsh
```

#### Debian / Ubuntu

```bash
sudo apt update
sudo apt install \
    git build-essential cmake ninja-build \
    qt6-base-dev qt6-tools-dev qtcreator \
    libopenblas-dev liblapack-dev \
    libocct-dev libgmsh-dev
```

#### Fedora

```bash
sudo dnf install \
    git gcc-c++ make cmake ninja-build \
    qt6-qtbase-devel qt6-qttools-devel qt-creator \
    openblas-devel lapack-devel \
    opencascade-devel gmsh-devel
```

#### openSUSE Tumbleweed

```bash
sudo zypper install \
    git gcc-c++ make cmake ninja \
    qt6-base-devel qt6-tools-devel qt-creator \
    openblas-devel lapack-devel \
    opencascade-devel gmsh-devel
```

### Clone the Repository

```bash
git clone https://github.com/techwinder/flow5.git
cd flow5
```

### Open the Project

Launch **Qt Creator** and open:

```text
flow5.pro
```

On first launch, Qt Creator will ask you to configure one or more build kits. Select a **Release** kit (Debug is optional) and choose a build directory outside of the source tree if possible.

### Configure Dependencies

If your dependencies are installed in non-standard locations, edit:

* `flow5-app.pro`
* `flow5-lib.pro`
* `flow5-io-lib.pro`

to point to the correct **include** and **library** directories for OpenCascade, Gmsh, and your linear algebra library.

### Build

Build the project using **Release** mode from Qt Creator.

Occasionally, the first build may fail if `flow5-app` is compiled before the internal libraries. Simply build the project again.

### Running

When launched from Qt Creator, the required library paths are configured automatically.

If launching Flow5 from a terminal, ensure your operating system can locate the required shared libraries (`.so` files). Some distributions may require adding `/usr/local/lib` (or another installation directory) to your library search path.

### Notes

* Flow5 has been tested with **OpenCascade 7.9.2** and **Gmsh 4.14.1**.
* OpenBLAS and Intel MKL are both supported for linear algebra.
* Building with Qt Creator is the recommended workflow for development and debugging.


### macOS

Download the latest macOS release from the **https://github.com/techwinder/flow5/releases** page, open the application 