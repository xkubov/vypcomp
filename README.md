# vypcomp: a compiler for VYPlanguage

VYPlanguage is a fancy language designed by Zbyněk Křivka, Radim Kocman, Dominika Regéciová at VUT FIT for the VYPa compiler class.

## Build and Installation

* `git clone https://github.com/xkubov/vypcomp`
* `mkdir build && cd build`
* `cmake .. -DCMAKE_INSTALL_PREFIX=${INSTALL}`
* `make -j$(nproc)`

To be able to debug during developement use another option `-DCMAKE_BUILD_TYPE=Debug` (Default is Release with agressive optimizations).

## Running tests

To run unit tests use:
`${INSTALL}/bin/vypcomp-tests`

## Running compiler.

NOTE: Section in progress.

After installation compiler is located in `${INSTALL}/bin/vypcomp`.
