#ifndef EBITEN_GRAPHICS_DETAIL_OPENGL_DEVICE_HPP
#define EBITEN_GRAPHICS_DETAIL_OPENGL_DEVICE_HPP

#include "ebiten/graphics/detail/opengl/graphics_context.hpp"
#include "ebiten/graphics/detail/opengl/opengl_initializer.hpp"
#include "ebiten/graphics/detail/opengl/texture_factory.hpp"
#include "ebiten/noncopyable.hpp"

#ifdef EBITEN_MACOSX
# include <OpenGL/gl.h>
#endif
#ifdef EBITEN_IOS
# import <GLKit/GLKit.h>
#endif

#include <cassert>
#include <functional>
#include <thread>

namespace ebiten {
namespace graphics {
namespace detail {

class device : private ebiten::noncopyable {
private:
  std::size_t const screen_width_;
  std::size_t const screen_height_;
  std::size_t const screen_scale_;
  std::function<void()> update_func_;
  std::function<void()> draw_func_;
  graphics_context graphics_context_;
  texture_factory texture_factory_;
  texture offscreen_texture_;
  GLuint offscreen_framebuffer_;
  std::mutex mutex_;
public:
  device(std::size_t screen_width,
         std::size_t screen_height,
         std::size_t screen_scale,
         view& view,
         std::function<void()> const& update_func,
         std::function<void()> const& draw_func)
    : screen_width_(screen_width),
      screen_height_(screen_height),
      screen_scale_(screen_scale),
      update_func_(update_func),
      draw_func_(draw_func) {
    assert(0 < this->screen_width_);
    assert(0 < this->screen_height_);
    assert(0 < this->screen_scale_);
    assert(this->update_func_);
    assert(this->draw_func_);
    opengl_initializer::initialize(view, std::bind(&device::update, this));
  }
  // TODO: destructor
  /*
   * NOTICE:
   *   The OpenGL functions should be called only in this method 'update'.
   *   Is that better to add an argument to this method?
   */
  void
  update() {
    std::lock_guard<std::mutex> lock(this->mutex_);
    // TODO: If application is terminated, stop
    if (!this->offscreen_texture_) {
      this->initialize_offscreen();
    }
    this->update_func_();
    ::glEnable(GL_TEXTURE_2D); // is not valid in OpenGL ES. Why?
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    {
      GLint origFramebuffer;
      ::glGetIntegerv(GL_FRAMEBUFFER_BINDING, &origFramebuffer);
      ::glBindFramebuffer(GL_FRAMEBUFFER, this->offscreen_framebuffer_);
      this->graphics_context_.clear();
      ::glEnable(GL_BLEND);
      ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      std::size_t const width  = this->offscreen_texture_.width();
      std::size_t const height = this->offscreen_texture_.height();
      ::glViewport(0, 0,
                   static_cast<GLsizei>(width),
                   static_cast<GLsizei>(height));
      float const projection_matrix[] = {
        2.0 / width, 0,            0, 0,
        0,           2.0 / height, 0, 0,
        0,           0,            1, 0,
        -1,          -1,           0, 1,
      };
      this->graphics_context_.set_projection_matrix(std::begin(projection_matrix),
                                                    std::end(projection_matrix));
      this->graphics_context_.reset_geometry_matrix();
      this->graphics_context_.reset_color_matrix();
      this->draw_func_();
      ::glFlush();
      ::glBindFramebuffer(GL_FRAMEBUFFER, origFramebuffer);
    }
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    this->graphics_context_.clear();
    ::glDisable(GL_BLEND);
    {
      std::size_t const width  = this->screen_width_  * this->screen_scale_;
      std::size_t const height = this->screen_height_ * this->screen_scale_;
      ::glViewport(0, 0,
                   static_cast<GLsizei>(width),
                   static_cast<GLsizei>(height));
      float const projection_matrix[] = {
        2.0 / width, 0,             0, 0,
        0,           -2.0 / height, 0, 0,
        0,           0,             1, 0,
        -1,          1,             0, 1,
      };
      this->graphics_context_.set_projection_matrix(std::begin(projection_matrix),
                                                    std::end(projection_matrix));
      geometry_matrix geometry_matrix(this->screen_scale_, 0,
                                      0, this->screen_scale_,
                                      0, 0);
      this->graphics_context_.set_geometry_matrix(geometry_matrix);
      this->graphics_context_.reset_color_matrix();
    }
    this->graphics_context_.set_texture(this->offscreen_texture_);
    this->graphics_context_.draw(0, 0, 0, 0, this->screen_width_, this->screen_height_);
    ::glFlush();
  }
  graphics_context&
  graphics_context() {
    return this->graphics_context_;
  }
  texture_factory&
  texture_factory() {
    return this->texture_factory_;
  }
private:
  void
  initialize_offscreen() {
    this->offscreen_texture_ = texture_factory().create(this->screen_width_,
                                                        this->screen_height_);
    ::glGenFramebuffers(1, &this->offscreen_framebuffer_);
    {
      GLint origFramebuffer;
      ::glGetIntegerv(GL_FRAMEBUFFER_BINDING, &origFramebuffer);
      ::glBindFramebuffer(GL_FRAMEBUFFER, this->offscreen_framebuffer_);
      ::glFramebufferTexture2D(GL_FRAMEBUFFER,
                               GL_COLOR_ATTACHMENT0,
                               GL_TEXTURE_2D,
                               this->offscreen_texture_.id(),
                               0);
      ::glBindFramebuffer(GL_FRAMEBUFFER, origFramebuffer);
    }
    if (::glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      throw std::runtime_error("framebuffer is not supported completely");
    }
  }
};

}
}
}

#endif
