#ifndef EBITEN_GAME_VIDEO_OPENGL_DEVICE_HPP
#define EBITEN_GAME_VIDEO_OPENGL_DEVICE_HPP

#include "ebiten/game/timer.hpp"
#include "ebiten/game/video/opengl/cocoa.hpp"
#include "ebiten/game/video/opengl/graphics_context.hpp"
#include "ebiten/game/video/opengl/texture_factory.hpp"
#include "ebiten/game/video/sprite.hpp"
#include "ebiten/util/singleton.hpp"
#include <boost/range.hpp>
#include <GLUT/glut.h>
#include <atomic>
#include <cassert>
#include <pthread.h>

namespace ebiten {
namespace game {
namespace video {
namespace opengl {

class device : public util::singleton<device> {
  friend class util::singleton<device>;
private:
  // TODO: move it into the 'run' function
  static std::function<void()> display_func_;
public:
  template<class Game>
  void
  run(Game& game,
      std::size_t screen_width,
      std::size_t screen_height,
      std::size_t fps,
      std::size_t window_scale) {
    assert(window_scale);
    // TODO: http://developer.apple.com/library/mac/#documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_general/opengl_gen_tasks.html#//apple_ref/doc/uid/TP40001987-CH211-SW1
    int argc = 1;
    char arg[] = "ebiten";
    char* argv[] = {arg, nullptr};
    ::glutInit(&argc, argv);
    ::glutInitWindowSize(screen_width * window_scale, screen_height * window_scale);
    ::glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
    ::glutCreateWindow("Ebiten");

    // offscreen
    auto& tf = texture_factory::instance();
    auto offscreen_texture = tf.create(screen_width, screen_height);
    GLuint framebuffer = 0;
    ::glGenFramebuffersEXT(1, &framebuffer);
    assert(framebuffer);
    ::glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
    ::glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT,
                                GL_COLOR_ATTACHMENT0_EXT,
                                GL_TEXTURE_2D,
                                offscreen_texture->id(),
                                0);
    if (::glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT) {
      throw "framebuffer is not supported completely";
    }
    ::glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
    
    int frame_index = 0;
    timer timer(fps);
    const float offscreen_width  = static_cast<float>(offscreen_texture->width());
    const float offscreen_height = static_cast<float>(offscreen_texture->height());
    const float offscreen_tu     = offscreen_width  / offscreen_texture->texture_width();
    const float offscreen_tv     = offscreen_height / offscreen_texture->texture_height();
    const float offscreen_vertex[4][3] = {{0,               0,                0},
                                          {offscreen_width, 0,                0},
                                          {offscreen_width, offscreen_height, 0},
                                          {0,               offscreen_height, 0}};
    const float window_scale_f = static_cast<float>(window_scale);
    const float offscreen_geo[] = {window_scale_f, 0,              0, 0,
                                   0,              window_scale_f, 0, 0,
                                   0,              0,              1, 0,
                                   0,              0,              0, 1};
    display_func_ = [&]{
      // render sprites to the offscreen
      ::glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, framebuffer);
      ::glClearColor(0, 0, 0, 1);
      ::glClear(GL_COLOR_BUFFER_BIT);        
      ::glEnable(GL_TEXTURE_2D);
      ::glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
      ::glEnable(GL_BLEND);
      ::glViewport(0, 0, screen_width, screen_height);
      ::glMatrixMode(GL_PROJECTION);
      ::glLoadIdentity();
      ::glOrtho(0, screen_width, 0, screen_height, 0, 1);
      const auto& sprites = game.sprites();
      typedef video::sprite sprite;
      typedef std::reference_wrapper<const sprite> sprite_cref;
      std::vector<sprite_cref> sorted_sprites;
      sorted_sprites.reserve(boost::size(sprites));
      std::for_each(boost::begin(sprites), boost::end(sprites),
                    [&](const sprite& s) {
                      sorted_sprites.emplace_back(s);
                    });
      // sort the sprites in desceinding order of z
      std::sort(sorted_sprites.begin(), sorted_sprites.end(),
                [](const sprite_cref& a, const sprite_cref& b) {
                  const double diff = a.get().z() - b.get().z();
                  return (0 < diff) ? -1 : ((diff < 0) ? 1 : 0);
                });
      std::for_each(boost::begin(sorted_sprites), boost::end(sorted_sprites),
                    [&](const sprite_cref& s) {
                      s.get().draw(graphics_context::instance());
                    });

      // start the logic loop
      struct logic_func {
        static void* invoke(void* func_ptr) {
          auto func = *(reinterpret_cast<std::function<void()>*>(func_ptr));
          func();
          return nullptr;
        }
      };
      std::atomic<bool> swap_completed(false);
      std::function<void()> logic_func = [&]{
        while (!swap_completed.load()) {
          timer.wait_frame();
          game.update(frame_index);
          ++frame_index;
        }
      };
      pthread_t logic_thread;
      ::pthread_create(&logic_thread, nullptr, logic_func::invoke, &logic_func);

      ::glFlush();
      ::glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);

      // render the offscreen to the screen
      ::glClearColor(0, 0, 0, 1);
      ::glClear(GL_COLOR_BUFFER_BIT);
      ::glEnable(GL_TEXTURE_2D);
      ::glDisable(GL_BLEND);
      ::glViewport(0, 0, screen_width * window_scale, screen_height * window_scale);
      ::glMatrixMode(GL_PROJECTION);
      ::glLoadIdentity();
      ::glOrtho(0, screen_width * window_scale, screen_height * window_scale, 0, 0, 1);
      ::glMatrixMode(GL_MODELVIEW);
      ::glLoadMatrixf(offscreen_geo);
      ::glBindTexture(GL_TEXTURE_2D, offscreen_texture->id());
      ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
      ::glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      {
        ::glBegin(GL_QUADS);
        ::glTexCoord2f(0, 0);
        ::glVertex3fv(offscreen_vertex[0]);
        ::glTexCoord2f(offscreen_tu, 0);
        ::glVertex3fv(offscreen_vertex[1]);
        ::glTexCoord2f(offscreen_tu, offscreen_tv);
        ::glVertex3fv(offscreen_vertex[2]);
        ::glTexCoord2f(0, offscreen_tv);
        ::glVertex3fv(offscreen_vertex[3]);
        ::glEnd();
      }
      ::glBindTexture(GL_TEXTURE_2D, 0);
      ::glutSwapBuffers();

      swap_completed.store(true);
      ::pthread_join(logic_thread, nullptr);

      ::glutPostRedisplay();
    };
    struct display_func {
      static void invoke() {
        display_func_();
      }
    };
    ::glutDisplayFunc(display_func::invoke);
    // converting lambda to a function pointer fails in a template member function
    // in gcc 4.6
    struct idle_func {
      static void invoke() {
        ::glutPostRedisplay();
      }
    };
    ::glutIdleFunc(idle_func::invoke);
    if (!cocoa::initialize()) {
      throw "ebiten::game::video::opengl::cocoa::initialize() was failed";
    }

    game.initialize(texture_factory::instance());
    ::glutMainLoop();
  }
private:
  device() {
  }
  ~device() {
  }
};

std::function<void()> device::display_func_;

}
}
}
}

#endif
