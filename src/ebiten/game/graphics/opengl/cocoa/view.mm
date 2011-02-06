#include "ebiten/game/graphics/opengl/cocoa/view.hpp"
#import "ebiten/game/graphics/opengl/cocoa/view.m"
#include <cassert>

namespace ebiten {
namespace game {
namespace graphics {
namespace opengl {
namespace cocoa {

namespace detail {

void
initialize(id window) {
  assert(window != nil);
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSRect rect = NSMakeRect(0, 0, 640, 480);
  NSOpenGLPixelFormatAttribute attributes[] = {
    NSOpenGLPFAWindow,
    NSOpenGLPFADoubleBuffer,
    NSOpenGLPFAAccelerated,
    NSOpenGLPFADepthSize, 16,
    nil,
  };
  NSOpenGLPixelFormat* format = [[[NSOpenGLPixelFormat alloc] initWithAttributes:attributes]
                                 autorelease];
  //NSOpenGLContext* context = [[NSOpenGLContext alloc] initWithFormat:format shareContext:nil];
  EbitenOpenGLView* glView = [[EbitenOpenGLView alloc] initWithFrame:rect pixelFormat:format];
  [window setContentView:glView];
  [pool release];
}

void
initialize(std::ptrdiff_t native_frame) {
  assert(native_frame);
  id window = reinterpret_cast<id>(native_frame);
  initialize(window);
}

}

}
}
}
}
}
