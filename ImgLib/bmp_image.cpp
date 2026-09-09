#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <cstdint>
#include <fstream>
#include <string_view>
#include <vector>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    char signature[2];
    uint32_t file_size;
    uint32_t reserved;
    uint32_t data_offset;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t header_size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t data_size;
    int32_t h_resolution;
    int32_t v_resolution;
    int32_t colors_used;
    int32_t significant_colors;
}
PACKED_STRUCT_END

static_assert(sizeof(BitmapFileHeader) == 14, "BitmapFileHeader must be packed to 14 bytes");
static_assert(sizeof(BitmapInfoHeader) == 40, "BitmapInfoHeader must be packed to 40 bytes");

static const int BMP_HEADERS_SIZE = 54;
static const uint16_t BMP_BIT_COUNT = 24;
static const int32_t BMP_RESOLUTION = 11811;

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

bool SaveBMP(const Path& file, const Image& image) {
    ofstream ofs(file, ios::binary);
    if (!ofs) {
        return false;
    }

    const int w = image.GetWidth();
    const int h = image.GetHeight();
    const int stride = GetBMPStride(w);

    BitmapFileHeader file_header;
    file_header.signature[0] = 'B';
    file_header.signature[1] = 'M';
    file_header.file_size = BMP_HEADERS_SIZE + static_cast<uint32_t>(stride) * h;
    file_header.reserved = 0;
    file_header.data_offset = BMP_HEADERS_SIZE;

    BitmapInfoHeader info_header;
    info_header.header_size = sizeof(BitmapInfoHeader);
    info_header.width = w;
    info_header.height = h;
    info_header.planes = 1;
    info_header.bit_count = BMP_BIT_COUNT;
    info_header.compression = 0;
    info_header.data_size = static_cast<uint32_t>(stride) * h;
    info_header.h_resolution = BMP_RESOLUTION;
    info_header.v_resolution = BMP_RESOLUTION;
    info_header.colors_used = 0;
    info_header.significant_colors = 0x1000000;

    ofs.write(reinterpret_cast<const char*>(&file_header), sizeof(file_header));
    ofs.write(reinterpret_cast<const char*>(&info_header), sizeof(info_header));
    if (!ofs) {
        return false;
    }

    // строка пишется целиком за одну операцию; хвостовые байты выравнивания
    // заполняются нулями один раз при создании буфера и больше не трогаются
    vector<char> buff(stride, 0);

    // строки в BMP хранятся снизу вверх
    for (int y = h - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);

        for (int x = 0; x < w; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }

        ofs.write(buff.data(), stride);
        if (!ofs) {
            return false;
        }
    }

    return true;
}

Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);
    if (!ifs) {
        return {};
    }

    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    ifs.read(reinterpret_cast<char*>(&file_header), sizeof(file_header));
    ifs.read(reinterpret_cast<char*>(&info_header), sizeof(info_header));
    if (!ifs) {
        return {};
    }

    if (file_header.signature[0] != 'B' || file_header.signature[1] != 'M') {
        return {};
    }

    if (info_header.header_size != sizeof(BitmapInfoHeader)
        || info_header.bit_count != BMP_BIT_COUNT
        || info_header.compression != 0
        || info_header.width <= 0
        || info_header.height <= 0) {
        return {};
    }

    const int w = info_header.width;
    const int h = info_header.height;
    const int stride = GetBMPStride(w);

    Image result(w, h, Color::Black());
    vector<char> buff(stride);

    // строки в BMP хранятся снизу вверх
    for (int y = h - 1; y >= 0; --y) {
        ifs.read(buff.data(), stride);
        if (!ifs) {
            return {};
        }

        Color* line = result.GetLine(y);

        for (int x = 0; x < w; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
            line[x].a = byte{255};
        }
    }

    return result;
}

}  // namespace img_lib
