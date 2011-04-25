#ifndef EBITEN_UTIL_IMAGE_LOADER_HPP
#define EBITEN_UTIL_IMAGE_LOADER_HPP

#include "ebiten/util/image.hpp" // TODO: remove it
#include <boost/gil/gil_all.hpp>
#include <boost/gil/extension/io/png_io.hpp>

namespace ebiten {
namespace util {

struct png_image_loader_t {
public:
  void
  operator()(std::string const& filename,
             std::size_t& width,
             std::size_t& height,
             std::vector<uint8_t>& pixels) const {
    typedef boost::gil::rgba8_image_t gil_image_t;
    gil_image_t gil_image;
    boost::gil::png_read_image(filename, gil_image);
    width  = gil_image.width();
    height = gil_image.height();
    pixels.resize(width * height * 4);
    boost::gil::rgba8_pixel_t* pixelsPtr =
      reinterpret_cast<boost::gil::rgba8_pixel_t*>(pixels.data());
    gil_image_t::view_t viewSrc =
      boost::gil::view(gil_image);
    gil_image_t::view_t viewDst =
      boost::gil::interleaved_view(width, height, pixelsPtr, width * 4);
    boost::gil::copy_pixels(viewSrc, viewDst);
  }
} png_image_loader = {};

}
}

#ifdef EBITEN_TEST

namespace ebiten {
namespace util {

TEST(image_loader, png_image_loader) {
  image img(png_image_loader, "test.png");
  EXPECT_EQ(32u, img.width());
  EXPECT_EQ(32u, img.height());
}

}
}

#endif

#endif

