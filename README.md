# FrictionQPotFEM

[![CI](https://github.com/tdegeus/FrictionQPotFEM/workflows/CI/badge.svg)](https://github.com/tdegeus/FrictionQPotFEM/actions)
[![Doxygen -> gh-pages](https://github.com/tdegeus/FrictionQPotFEM/workflows/gh-pages/badge.svg)](https://tdegeus.github.io/FrictionQPotFEM)
[![Conda Version](https://img.shields.io/conda/vn/conda-forge/frictionqpotfem.svg)](https://anaconda.org/conda-forge/frictionqpotfem)

Friction simulations based on "GMatElastoPlasticQPot" and "GooseFEM"

## Testing

## Additional checks and balances

Additionally, consistency against earlier runs can be checked as follows.

### UniformSingleLayer2d - PNAS

```none
cd build
cmake .. -DBUILD_TESTS=1 -DBUILD_EXAMPLES=1
make
./examples/PNAS-2019 ../examples/PNAS-2019_N=3\^2_id=000.h5
```

### UniformSingleLayer2d::System

```none
cd build
cmake .. -DBUILD_TESTS=1 -DBUILD_EXAMPLES=1
make
./examples/UniformSingleLayer2d_System
cp ../examples/UniformSingleLayer2d_System.py .
python UniformSingleLayer2d_System.py ../examples/UniformSingleLayer2d_System.txt
```

### UniformSingleLayer2d::HybridSystem

```none
cd build
cmake .. -DBUILD_TESTS=1 -DBUILD_EXAMPLES=1
make
./examples/UniformSingleLayer2d_HybridSystem
cp ../examples/UniformSingleLayer2d_HybridSystem.py .
python UniformSingleLayer2d_HybridSystem.py ../examples/UniformSingleLayer2d_HybridSystem.txt
```

### UniformSingleLayer2d - Load cycle

```none
cd build
cmake .. -DBUILD_TESTS=1 -DBUILD_EXAMPLES=1
make
./examples/LoadCycle
cp ../examples/LoadCycle.py .
python LoadCycle.py ../examples/LoadCycle.txt
```

## Generating the docs

### Basic

```
cd docs
doxygen
```

### With dependencies

For example using conda

```
cd docs
( cat Doxyfile ; echo "INPUT += ${CONDA_PREFIX}/include/GooseFEM" ; echo "INPUT += ${CONDA_PREFIX}/include/GMatElastoPlasticQPot" ) | doxygen -
```

## Change-log

### v0.9.1

*   Python API: forcing copy of certain objects (#62)
*   Using CMake for Doxygen (#61, #62)
*   Adding dependencies to docs

### v0.9.0

*   Adding convenience method "Energy"
*   Removing namespace aliases
*   Getting mass and damping matrix

### v0.8.0

*   Updating versioning. Python API: auto-overloading (#57)
*   Using setuptools_scm to manage version (#56)
*   Documenting version information. Adding eigen to version string.
*   Renaming "versionInfo" -> "version_dependencies"
*   Adding fdamp and setV/setA
*   [CI] Switching to GCC 8 for the moment (#51)
*   Examples: modifying to API change, removing namespace abbreviation.
*   Removing GoosFEM alias

### v0.7.0

*   Adding addAffineSimpleShearCentered (#41)

### v0.6.0

*   [CI] Removing travis and appveyor
*   Adding possibility to restart (#40)
*   Clang/Windows switching-off xtensor::optimize (#39)
*   Getting plastic strain (#38)
*   Reformatting CI: using clang on Windows
*   Compute sign of displacement perturbation
*   Add affine shear step
*   Adding "currentYield*" offset overload
*   LocalTrigger: adding option to evaluate only small slice; making energy relative to the  
    volume of a plastic element (not to that of the system); adding simple-shear search
*   Splitting tests in several sources, enable Windows CI
*   Trigger: most output
*   Triggering: optimisations & minimal search (#34)
    -   Trigger: Optimizations
    -   Adding minimal search
    -   Changing call of parent constructor
*   Deprecated local energy barriers (#33)
*   Implementation triggering of smallest energy barrier (#31)
    -   API Change: Removing "init" functions, using constructors directly. 
        Adding stiffness matrix.
    -   Adding triggering protocol
*    Exploring energy barrier upon trigger (#30)
*    Adding plastic_ElementYieldBarrierForSimpleShear (#28)
*    Exploring energy landscape to local simple shear perturbation (#24)
*    Temporarily excluding Windows from GitHub CI
*    Adding GitHub CI
*    Adding Python API
*    Amplifying trigger
