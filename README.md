# C++ Image Converter

A command-line image converter written in C++20. It reads and writes BMP, JPEG,
and PPM images through a reusable image library and selects the appropriate
codec from each file extension.

## Highlights

- Converts between BMP, JPEG, and PPM in any supported combination.
- Implements 24-bit BMP encoding and decoding without a third-party BMP
  library.
- Uses packed 14-byte and 40-byte BMP headers with compile-time size checks.
- Handles bottom-up BMP scanlines, BGR/RGB conversion, and four-byte row
  alignment.
- Buffers an entire scanline per I/O operation instead of reading or writing
  individual pixels.
- Uses polymorphism to keep format selection separate from conversion logic.
- Builds the image library as a static CMake target.
- Reports invalid formats and file-loading or file-saving failures through
  distinct exit codes.

## Architecture

The project consists of two components:

- `ImgLib` contains the `Image` and `Color` types plus BMP, JPEG, and PPM
  codecs.
- `ImgConverter` provides the `imgconv` command-line application. Its
  `ImageFormatInterface` adapters expose each codec through the same loading
  and saving API.

```text
cpp-image-converter/
├── ImgLib/
│   ├── img_lib.h / img_lib.cpp
│   ├── bmp_image.h / bmp_image.cpp
│   ├── jpeg_image.h / jpeg_image.cpp
│   ├── ppm_image.h / ppm_image.cpp
│   └── CMakeLists.txt
└── ImgConverter/
    ├── main.cpp
    └── CMakeLists.txt
```

## Supported formats

| Format | Extensions | Implementation |
| --- | --- | --- |
| BMP | `.bmp` | Native 24-bit uncompressed BMP codec |
| JPEG | `.jpg`, `.jpeg` | libjpeg |
| PPM | `.ppm` | Native binary P6 codec |

Format detection is based on the input and output filename extensions.

## Requirements

- A C++20-compatible compiler
- CMake 3.11 or newer
- libjpeg headers and library

The CMake configuration expects the directory supplied through `LIBJPEG_DIR`
to contain:

```text
<LIBJPEG_DIR>/
├── include/
└── lib/
    ├── Debug/
    └── Release/
```

## Build

From the repository root:

```bash
cmake -S ImgConverter -B build \
  -DLIBJPEG_DIR="/path/to/libjpeg" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

For a Visual Studio multi-configuration generator, omit
`-DCMAKE_BUILD_TYPE=Release` and keep `--config Release` in the build command.

The executable is generated as `imgconv` (`imgconv.exe` on Windows). Depending
on the selected CMake generator, it will be located in either `build/` or
`build/Release/`.

## Usage

```text
imgconv <input-file> <output-file>
```

The desired conversion is inferred from the two extensions:

```bash
imgconv photo.jpg photo.bmp
imgconv photo.bmp photo.ppm
imgconv photo.ppm photo.jpg
```

On success, the application prints:

```text
Successfully converted
```

## Implementation notes

The BMP codec validates the file signature and supported header properties
before allocating an image. It reads and writes complete padded scanlines,
converts between BMP's BGR byte order and the library's RGBA pixel
representation, and reverses row order because standard BMP data is stored
bottom-up.

The converter itself does not contain format-specific conversion branches.
It resolves an input and output implementation through
`ImageFormatInterface`, loads one in-memory `Image`, and passes it to the
selected output codec. Adding another format therefore only requires a codec
and a matching adapter.

## Error codes

| Code | Meaning |
| ---: | --- |
| `0` | Conversion completed successfully |
| `1` | Invalid command-line arguments |
| `2` | Unsupported input extension |
| `3` | Unsupported output extension |
| `4` | Input image could not be loaded |
| `5` | Output image could not be saved |

## Technology

C++20, CMake, STL, libjpeg, binary file I/O, static libraries, and
object-oriented format abstraction.
