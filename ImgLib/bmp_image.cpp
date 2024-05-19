#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

static const std::array<uint8_t, 2> BMP_SIG{ 'B', 'M' };

PACKED_STRUCT_BEGIN BitmapFileHeader {
    // поля заголовка Bitmap File Header
    std::array<uint8_t, 2> signature{ 'B', 'M' }; // подпись
    uint32_t size = 0; // суммарный размер заголовка и данных
    uint32_t reserve = 0; // зарезервированное пространство
    uint32_t indent = 54; // отступ данных от начала файла
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    // поля заголовка Bitmap Info Header
    uint32_t size = 40; // размер заголовка
    int32_t width = 0; // ширина изображения в пикселях
    int32_t height = 0; // высота изображения в пикселях
    uint16_t planes = 1; // количество плоскостей
    uint16_t bit_pixel = 24; // количество бит на пиксель
    uint32_t compression = 0; // тип сжатия
    uint32_t bytes = 0; // количество байт в данных
    int32_t horizontal_res = 11811; // горизонтальное разрешение, пикселей на метр
    int32_t vertical_res = 11811; // вертикальное разрешение, пикселей на метр
    int32_t numb_colors_used = 0; // количество использованных цветов
    int32_t sign_сolors = 0x1000000; // количество значимых цветов
}
PACKED_STRUCT_END

// функция вычисления отступа по ширине
static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

bool SaveBMP(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);
    if (!out) {
        return false;
    }

    const int width = image.GetWidth();
    const int height = image.GetHeight();
    const int bmp_stride = GetBMPStride(width);

    BitmapFileHeader file_header;
    file_header.size = sizeof(BitmapFileHeader) + sizeof(BitmapInfoHeader) + bmp_stride * height;

    BitmapInfoHeader info_header;
    info_header.width = width;
    info_header.height = height;
    info_header.bytes = bmp_stride * height;

    out.write(reinterpret_cast<const char*>(&file_header), sizeof(BitmapFileHeader));
    out.write(reinterpret_cast<const char*>(&info_header), sizeof(BitmapInfoHeader));

    vector<char> buf(bmp_stride);

    for (int y = height - 1; y >= 0; --y) {
        for (int x = 0; x < width; ++x) {
            const auto& pixel = image.GetPixel(x, y);
            buf[x * 3] = static_cast<char>(pixel.b);
            buf[x * 3 + 1] = static_cast<char>(pixel.g);
            buf[x * 3 + 2] = static_cast<char>(pixel.r);
        }
        out.write(buf.data(), bmp_stride);
    }

    return out.good();
}

Image LoadBMP(const Path& file) {
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;

    ifstream ifs(file, ios::binary);
    if (!ifs) {
        return {};
    }
    ifs.read(reinterpret_cast<char*>(&file_header), sizeof(BitmapFileHeader));
    ifs.read(reinterpret_cast<char*>(&info_header), sizeof(BitmapInfoHeader));
    if (file_header.signature != BMP_SIG) {
        return {};
    }

    const int width = info_header.width;
    const int height = info_header.height;
    const int bmp_stride = GetBMPStride(width);

    Image result(width, height, Color::Black());
    vector<char> buf(bmp_stride);

    for (int y = height - 1; y >= 0; --y) {
        ifs.read(buf.data(), bmp_stride);
        for (int x = 0; x < width; ++x) {
            auto& pixel = result.GetPixel(x, y);
            pixel.b = static_cast<byte>(buf[x * 3]);
            pixel.g = static_cast<byte>(buf[x * 3 + 1]);
            pixel.r = static_cast<byte>(buf[x * 3 + 2]);
            }
        }
        return result;
}

}  // namespace img_lib