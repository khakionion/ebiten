#ifndef EBINTE_SCRIPT_V8_GAME_HPP
#define EBINTE_SCRIPT_V8_GAME_HPP

#include "ebiten/graphics/graphics_context.hpp"
#include "ebiten/graphics/texture_factory.hpp"
#include "ebiten/input.hpp"
#include "ebiten/noncopyable.hpp"

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-parameter"
#include "v8.h"
#pragma clang diagnostic pop

#include <string>

namespace ebiten_script {
namespace v8 {

class game : private ebiten::noncopyable {
public:
  game(std::string) {
  }
  ~game() {
  }
  bool
  update(ebiten::graphics::texture_factory&,
         ebiten::input const&) {
    return false;
  }
  void
  draw(ebiten::graphics::graphics_context&,
       ebiten::graphics::texture&) {
  }
  void
  set_terminated_handler(std::function<void()> const&) {
    //this->terminated_handler_ = terminated_handler;
  }
  bool
  is_terminated() const {
    return false;
  }
};

}
}

#endif
