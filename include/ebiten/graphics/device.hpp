#ifndef EBITEN_GRAPHICS_DEVICE_HPP
#define EBITEN_GRAPHICS_DEVICE_HPP

#include "ebiten/platform.hpp"

#if defined(EBITEN_MACOSX) || defined(EBITEN_IOS)
# include "ebiten/graphics/detail/opengl/device.hpp"
#endif

namespace ebiten {
namespace graphics {

typedef detail::device device;

}
}

#endif