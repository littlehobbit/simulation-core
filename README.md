# simulation-core
Tool for creating NS-3 topologies dynamicly using XML profile.

## Installation
To build and install tool you need to install dependencies:
  - [`CLI11`](https://github.com/CLIUtils/CLI11)
  - [`ns-3`](https://gitlab.com/nsnam/ns-3-dev)
  - `fmt` - `apt install libfmt-dev`

To build and run tests, you need install `googletests` package.

```console
apt install libgtest-dev
```

```bash
mkdir build
cd build && cmake .. && cmake --build . --target all
```

Use cmake option `-DBUILD_TESTS=ON` to build tests.

## How To Use
More detailed information about XMl tags and format described in `doc/xml-format.md`.

### Examples
Examples are stored in `examples` directory.

To run example you can use
```bash
./simulation --xml ./examples/udp_echo.xml
```