#ifndef EBITEN_GRAPHICS_DETAIL_OPENGL_MACOSX_VIEW_MM
#define EBITEN_GRAPHICS_DETAIL_OPENGL_MACOSX_VIEW_MM

/*
 * Reference:
 *   http://developer.apple.com/library/mac/#qa/qa1385/_index.html
 *   http://www.alecjacobson.com/weblog/?p=2185
 */

#import <Cocoa/Cocoa.h>
#import <QuartzCore/QuartzCore.h>
#include <functional>

@interface EbitenOpenGLView : NSOpenGLView {
@private
  CVDisplayLinkRef displayLink_;
  std::function<void()> updatingFunc_;
}

- (CVReturn)getFrameForTime:(CVTimeStamp const*)outputTime;
- (void)setUpdatingFunc:(std::function<void()> const&)updatingFunc;
- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (void)mouseDown:(NSEvent*)theEvent;
- (void)keyDown:(NSEvent*)theEvent;

@end

static CVReturn
EbitenDisplayLinkCallback(CVDisplayLinkRef displayLink,
                          CVTimeStamp const* now,
                          CVTimeStamp const* outputTime,
                          CVOptionFlags flagsIn,
                          CVOptionFlags* flagsOut,
                          void* displayLinkContext) {
  (void)displayLink;
  (void)now;
  (void)flagsIn;
  (void)flagsOut;
  @autoreleasepool {
    EbitenOpenGLView* view = (__bridge EbitenOpenGLView*)displayLinkContext;
    return [view getFrameForTime:outputTime];
  }
}

#ifndef EBITEN_WITHOUT_OBJC_IMPL
@implementation EbitenOpenGLView

- (void)dealloc {
  CVDisplayLinkRelease(displayLink_);
  // Do not call [super dealloc] because of ARC.
}

- (void)prepareOpenGL {
  NSOpenGLContext* openGLContext = [self openGLContext];
  assert(openGLContext != nil);
  GLint const swapInterval = 1;
  [openGLContext setValues:&swapInterval
              forParameter:NSOpenGLCPSwapInterval]; 
  CVDisplayLinkCreateWithActiveCGDisplays(&displayLink_);
  CVDisplayLinkSetOutputCallback(displayLink_,
                                 &EbitenDisplayLinkCallback,
                                 (__bridge void*)self);
  CGLContextObj cglContext = (CGLContextObj)[openGLContext CGLContextObj];
  CGLPixelFormatObj cglPixelFormat = (CGLPixelFormatObj)[[self pixelFormat] CGLPixelFormatObj];
  CVDisplayLinkSetCurrentCGDisplayFromOpenGLContext(displayLink_, cglContext, cglPixelFormat);
  CVDisplayLinkStart(displayLink_);
}

- (CVReturn)getFrameForTime:(CVTimeStamp const*)outputTime {
  (void)outputTime;
  if (!self->updatingFunc_) {
    return kCVReturnSuccess;
  }
  NSOpenGLContext* context = [self openGLContext];
  assert(context != nil);
  [context makeCurrentContext];
  CGLLockContext((CGLContextObj)[context CGLContextObj]);
  self->updatingFunc_();
  [context flushBuffer];
  CGLUnlockContext((CGLContextObj)[context CGLContextObj]);
  return kCVReturnSuccess;
}

- (void)setUpdatingFunc:(std::function<void()> const&)updatingFunc {
  self->updatingFunc_ = updatingFunc;
}

- (BOOL)acceptsFirstResponder {
  return YES;
}

- (BOOL)becomeFirstResponder {
  return YES;
}

- (void)mouseDown:(NSEvent*)theEvent {
  NSPoint location = [self convertPoint:[theEvent locationInWindow] fromView:nil];
  NSLog(@"%@", NSStringFromPoint(location));
}

- (void)keyDown:(NSEvent*)theEvent {
  NSString* chars = [theEvent characters];
  NSLog(@"%@", chars);
}

@end
#endif

#endif
