# liquid-resize

A small CLI program to demonstrate the [seam carving algorithm](https://en.wikipedia.org/wiki/Seam_carving).

### Compilation
The project is compiled with [meson](https://mesonbuild.com/). In the root directory, run:
```bash
meson setup buildir
cd builddir
meson compile
```

### Usage
In your newly set up build directory, run the program:
```bash
./liquid-resize 80 example.jpeg
```

### Examples
![Original water](examples/water.jpeg?sanitize=true)
```bash
./liquid-resize 50 water.jpeg
```
![After resize](examples/output.jpg?sanitize=true)

