#ifndef EBITEN_GRAPHICS_DETAIL_OPENGL_GRAPHICS_CONTEXT_HPP
#define EBITEN_GRAPHICS_DETAIL_OPENGL_GRAPHICS_CONTEXT_HPP

#include "ebiten/graphics/detail/opengl/device.hpp"
#include "ebiten/graphics/color_matrix.hpp"
#include "ebiten/graphics/drawing_region.hpp"
#include "ebiten/graphics/geometry_matrix.hpp"
#include "ebiten/graphics/texture.hpp"
#include "ebiten/noncopyable.hpp"
#include <OpenGL/gl.h>
#include <array>
#include <algorithm>
#include <cassert>
#include <iostream>

namespace ebiten {
namespace graphics {
namespace detail {

class graphics_context : private noncopyable {
  friend class device;
private:
  GLuint shader_program;
  texture_id current_texture_id;
  std::size_t current_texture_width;
  std::size_t current_texture_height;
private:
  graphics_context()
    : shader_program(0) {
  }
public:
  void
  set_texture(texture_id texture_id,
              std::size_t texture_width,
              std::size_t texture_height) {
    this->current_texture_id     = texture_id;
    this->current_texture_width  = texture_width;
    this->current_texture_height = texture_height;
    ::glBindTexture(GL_TEXTURE_2D, texture_id);
  }
  void
  reset_texture() {
    ::glBindTexture(GL_TEXTURE_2D, 0);
  }
  void
  draw(drawing_region const& dr) {
    // TODO: replace float to short?
    // http://objective-audio.jp/2009/07/ngmoco-opengl.html
    // 選べるようにするといいかも
    float const texture_width  = current_texture_width;
    float const texture_height = current_texture_height;
    float const tu1 = dr.src_x                 / texture_width;
    float const tu2 = (dr.src_x + dr.width)  / texture_width;
    float const tv1 = dr.src_y                 / texture_height;
    float const tv2 = (dr.src_y + dr.height) / texture_height;
    float const x1 = dr.dst_x;
    float const x2 = dr.dst_x + dr.width;
    float const y1 = dr.dst_y;
    float const y2 = dr.dst_y + dr.height;
    float const vertex[4][3] = {{x1, y1},
                                {x2, y1},
                                {x2, y2},
                                {x1, y2}};
    // TODO: use glDrawArrays?
    // TODO: use glMultiDrawArrays?
    ::glBegin(GL_QUADS);
    ::glTexCoord2f(tu1, tv1);
    ::glVertex2fv(vertex[0]);
    ::glTexCoord2f(tu2, tv1);
    ::glVertex2fv(vertex[1]);
    ::glTexCoord2f(tu2, tv2);
    ::glVertex2fv(vertex[2]);
    ::glTexCoord2f(tu1, tv2);
    ::glVertex2fv(vertex[3]);
    ::glEnd();
  }
  void
  set_geometry_matrix(geometry_matrix const& mat) {
    float const gl_geo_mat[] = {mat.a(),  mat.c(),  0, 0,
                                mat.b(),  mat.d(),  0, 0,
                                0,        0,        1, 0,
                                mat.tx(), mat.ty(), 0, 1};
    ::glMatrixMode(GL_MODELVIEW);
    ::glLoadMatrixf(gl_geo_mat);
  }
  void
  reset_geometry_matrix() {
    ::glMatrixMode(GL_MODELVIEW);
    ::glLoadIdentity();
  }
  void
  set_color_matrix(color_matrix const& mat) {
    if (mat.is_identity()) {
      ::glUseProgram(0);
      return;
    }
    if (!this->shader_program) {
      this->shader_program = compile_shader_program();
      assert(this->shader_program);
    }
    ::glUseProgram(this->shader_program);
    ::glUniform1i(glGetUniformLocation(this->shader_program, "texture"), 0);
    float const gl_color_mat[] = {
      static_cast<float>(mat.element<0, 0>()),
      static_cast<float>(mat.element<0, 1>()),
      static_cast<float>(mat.element<0, 2>()),
      static_cast<float>(mat.element<0, 3>()),
      static_cast<float>(mat.element<1, 0>()),
      static_cast<float>(mat.element<1, 1>()),
      static_cast<float>(mat.element<1, 2>()),
      static_cast<float>(mat.element<1, 3>()),
      static_cast<float>(mat.element<2, 0>()),
      static_cast<float>(mat.element<2, 1>()),
      static_cast<float>(mat.element<2, 2>()),
      static_cast<float>(mat.element<2, 3>()),
      static_cast<float>(mat.element<3, 0>()),
      static_cast<float>(mat.element<3, 1>()),
      static_cast<float>(mat.element<3, 2>()),
      static_cast<float>(mat.element<3, 3>()),
    };
    ::glUniformMatrix4fv(glGetUniformLocation(this->shader_program, "color_matrix"),
                         1, GL_FALSE, gl_color_mat);
    float const gl_color_mat_translation[] = {
      static_cast<float>(mat.element<0, 4>()),
      static_cast<float>(mat.element<1, 4>()),
      static_cast<float>(mat.element<2, 4>()),
      static_cast<float>(mat.element<3, 4>()),
    };
    ::glUniform4fv(glGetUniformLocation(this->shader_program, "color_matrix_translation"),
                   4, gl_color_mat_translation);
  }
  void
  reset_color_matrix() {
    ::glUseProgram(0);
  }
private:
  GLuint
  compile_shader_program() {
    static std::string const sharder_source("uniform sampler2D texture;\n"
                                            "uniform mat4 color_matrix;\n"
                                            "uniform vec4 color_matrix_translation;\n"
                                            "\n"
                                            "void main(void) {\n"
                                            "  vec4 color = texture2DProj(texture, gl_TexCoord[0]);\n"
                                            "  gl_FragColor = color_matrix * color + color_matrix_translation;\n"
                                            "}\n");
    // TODO: ARB?
    GLuint fragment_shader;
    fragment_shader = ::glCreateShader(GL_FRAGMENT_SHADER);
    assert(fragment_shader);
    char const* shader_source_p = sharder_source.c_str();
    ::glShaderSource(fragment_shader, 1, &shader_source_p, 0);
    ::glCompileShader(fragment_shader);
    // check status
    GLint compiled;
    ::glGetShaderiv(fragment_shader, GL_COMPILE_STATUS, &compiled);
    this->show_shader_log(fragment_shader);
    if (compiled == GL_FALSE) {
      throw std::runtime_error("shader compile error");
    }

    GLuint program;
    program = ::glCreateProgram();
    assert(program);
    ::glAttachShader(program, fragment_shader);
    ::glDeleteShader(fragment_shader);
    ::glLinkProgram(program);
    // check status
    GLint linked;
    ::glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked == GL_FALSE) {
      throw std::runtime_error("program error");
    }

    return program;
  }
  // TODO: Debug Mode
  void
  show_shader_log(GLuint shader) {
    int log_size = 0;
    ::glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &log_size);
    if (log_size) {
      int length = 0;
      // TODO: 動的確保のほうがよい?
      std::array<char, 1024> buffer;
      // TODO: バッファ確認
      ::glGetShaderInfoLog(shader, buffer.size(), &length, buffer.data());
      std::cerr << buffer.data() << std::endl;
    }
  }
  // TODO: show_program_log
};

}
}
}

#endif