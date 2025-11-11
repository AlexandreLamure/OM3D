#ifndef IMAGEFORMAT_H
#define IMAGEFORMAT_H

#include <utils.h>

namespace OM3D {

enum class ImageFormat {
    RGBA8_UNORM,
    RGBA8_sRGB,
    RGB8_UNORM,
    RGB8_sRGB,

    RGBA16_FLOAT,
    Depth32_FLOAT
};


struct ImageFormatGL {
    u32 format;
    u32 internal_format;
    u32 component_type;
};

ImageFormatGL image_format_to_gl(ImageFormat format);

}

#endif // IMAGEFORMAT_H
