#ifndef EBITEN_SCRIPT_SQUIRREL_GAME_HPP
#define EBITEN_SCRIPT_SQUIRREL_GAME_HPP

#include "ebiten_script/sprite.hpp"
#include "ebiten_script/squirrel/geometry_matrix_funcs.hpp"
#include "ebiten_script/squirrel/texture_funcs.hpp"
#include "ebiten/graphics/graphics_context.hpp"
#include "ebiten/graphics/texture_factory.hpp"
#include "ebiten/noncopyable.hpp"
#include <squirrel.h> 
#include <sqstdio.h> 
#include <sqstdaux.h>
#include <cstdio>
#include <exception>
#include <string>
#include <iostream>

namespace ebiten_script {
namespace squirrel {

class game : private ebiten::noncopyable {
private:
  HSQUIRRELVM const vm_;
  SQInteger top_;
  HSQOBJECT game_;
public:
  game(std::string filename)
    : vm_(::sq_open(1024)) {
    ::sqstd_seterrorhandlers(this->vm_);
    ::sq_setprintfunc(this->vm_, print_func, error_func);
    // TODO: load the standard library
    this->top_ = ::sq_gettop(this->vm_);
    this->initialize_ebiten_classes();
    ::sq_pushroottable(this->vm_);
    ::sqstd_dofile(this->vm_,
                   _SC(filename.c_str()),
                   SQTrue,
                   SQTrue);
    if (::sq_gettype(this->vm_, -1) == OT_INSTANCE) {
      ::sq_getstackobj(this->vm_, -1, &this->game_);
    }
  }
  virtual
  ~game() {
    ::sq_settop(this->vm_, this->top_);
    ::sq_close(this->vm_);
  }
  void
  update(ebiten::graphics::texture_factory& tf) {
    {
      SQInteger const top = ::sq_gettop(this->vm_);
      ::sq_pushobject(this->vm_, this->game_);
      ::sq_pushstring(this->vm_, _SC("update"), -1);
      ::sq_get(this->vm_, -2);
      ::sq_pushobject(this->vm_, this->game_);
      ::sq_call(this->vm_, 1, SQFalse, SQTrue);
      ::sq_settop(this->vm_, top);
    }
    texture_funcs::instantiate(this->vm_, tf);
  }
  void
  draw(ebiten::graphics::graphics_context&) {
    {
      SQInteger const top = ::sq_gettop(this->vm_);
      ::sq_pushobject(this->vm_, this->game_);
      ::sq_pushstring(this->vm_, _SC("draw"), -1);
      ::sq_get(this->vm_, -2);
      ::sq_pushobject(this->vm_, this->game_);
      ::sq_call(this->vm_, 1, SQFalse, SQTrue);
      ::sq_settop(this->vm_, top);
    }
  }
private:
  static void
  print_func(HSQUIRRELVM, SQChar const* s, ...) {
    va_list vl;
    ::va_start(vl, s);
    ::vfprintf(stdout, s, vl);
    ::va_end(vl);
  }
  static void
  error_func(HSQUIRRELVM, SQChar const* s, ...) {
    va_list vl;
    ::va_start(vl, s);
    ::vfprintf(stderr, s, vl);
    ::va_end(vl);
    // TODO: throw?
    std::abort();
  }
  void
  initialize_ebiten_classes() {
    static std::string const e("ebiten");
    this->create_table(e);
    {
      static std::string const gm("GeometryMatrix");
      this->create_class(e, gm);
      this->create_method(e, gm,
                          "constructor",
                          geometry_matrix_funcs::constructor,
                          "x",
                          false);
      this->create_method(e, gm,
                          "_tostring",
                          geometry_matrix_funcs::meta_tostring,
                          "x",
                          false);
    }
    {
      static std::string const t("Texture");
      this->create_class(e, t);
      this->create_method(e, t,
                          "constructor",
                          texture_funcs::method_constructor,
                          "",
                          false);
    }
  }
  void
  create_table(std::string const& name) {
    SQInteger const top = ::sq_gettop(this->vm_);
    ::sq_pushroottable(this->vm_);
    ::sq_pushstring(this->vm_, _SC(name.c_str()), -1);
    ::sq_newtable(this->vm_);
    ::sq_newslot(this->vm_, -3, SQFalse);
    ::sq_settop(this->vm_, top);
  }
  void
  create_class(std::string const& namespace_name,
               std::string const& class_name) {
    SQInteger const top = ::sq_gettop(this->vm_);
    ::sq_pushroottable(this->vm_);
    ::sq_pushstring(this->vm_, _SC(namespace_name.c_str()), -1);
    ::sq_get(this->vm_, -2);
    ::sq_pushstring(this->vm_, _SC(class_name.c_str()), -1);
    ::sq_newclass(this->vm_, SQFalse);
    ::sq_newslot(this->vm_, -3, SQFalse);
    ::sq_settop(this->vm_, top);
  }
  void
  create_method(std::string const& namespace_name,
                std::string const& class_name,
                std::string const& method_name,
                SQFUNCTION func,
                std::string const& type_mask,
                bool is_static) {
    SQInteger const top = ::sq_gettop(this->vm_);
    ::sq_pushroottable(this->vm_);
    ::sq_pushstring(this->vm_, _SC(namespace_name.c_str()), -1);
    ::sq_get(this->vm_, -2);
    ::sq_pushstring(this->vm_, _SC(class_name.c_str()), -1);
    ::sq_get(this->vm_, -2);
    ::sq_pushstring(this->vm_, _SC(method_name.c_str()), -1);
    ::sq_newclosure(this->vm_, func, 0);
    if (!type_mask.empty()) {
      ::sq_setparamscheck(this->vm_, type_mask.size(), _SC(type_mask.c_str()));
    }
    ::sq_newslot(this->vm_, -3, is_static);
    ::sq_settop(this->vm_, top);
  }
};

}
}

#endif