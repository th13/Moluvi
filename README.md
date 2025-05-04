# Μολύβι

Moluvi ✏️ is a software-rendered 2D/3D graphics library implemented in pure C as an experimental/educational project.

## Examples

Here are some current examples that are rendered completely on CPU to a Moluvi `Canvas` before being sent to a Raylib `Texture2D` and rendered to screen.

![Rainbox](https://github.com/th13/Moluvi/blob/master/examples/rainbox.gif)
![Triangles](https://github.com/th13/Moluvi/blob/master/examples/triangles.gif)
![Sheets](https://github.com/th13/Moluvi/blob/master/examples/sheets.gif)

## Building from source

Moluvi uses the [nob.h](https://github.com/tsoding/nob.h) build system. The current primary entrypoint is `src/example.c` and `src/test.c` for tests.

> [!NOTE]
> Nob requires a one-time bootstrap build.
> Run `clang -o nob nob.c` to bootstrap. Subsequent invocations can simply run `nob`, which is smart enough to rebuild itself.

### Running the example 

`./nob example && ./build/example`

### Running the tests

`./nob test && ./build/test`

The tests load PPM files in the `test/` directory and for each test case generate a diff'd `Canvas` highlighting differences in the generated test from the source. If a diff is generated, the source canvas is first converted to greyscale and diff'd pixels are rendered in red.

