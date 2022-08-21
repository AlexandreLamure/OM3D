#include "ImageFormat.h"

#include <glad/glad.h>

ImageFormatGL image_format_to_gl(ImageFormat format) {
    switch(format) {
        case ImageFormat::RGBA8_UNORM: return ImageFormatGL{ GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE };
        case ImageFormat::Depth32_FLOAT: return ImageFormatGL{ GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32, GL_FLOAT };
    }

    FATAL("Unknown image format");
}
