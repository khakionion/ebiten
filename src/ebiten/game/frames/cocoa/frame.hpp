#ifndef EBITEN_GAME_FRAMES_FRAME_HPP
#define EBITEN_GAME_FRAMES_FRAME_HPP

#include "ebiten/util/id.hpp"
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <cstddef>

namespace ebiten {
namespace game {
namespace frames {
namespace cocoa {

class frame : private boost::noncopyable {
public:
  frame(std::size_t width, std::size_t height);
  ~frame();
  const util::id_& native_frame() const;
  std::size_t width() const {
    return this->width_;
  }
  std::size_t height() const {
    return this->height_;
  }
private:
  std::size_t width_;
  std::size_t height_;
  util::id_ window_;
};

}
}
}
}

#endif
