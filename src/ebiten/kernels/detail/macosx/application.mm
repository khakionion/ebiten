#ifndef EBITEN_KERNELS_DETAIL_MACOSX_APPLICATION_MM
#define EBITEN_KERNELS_DETAIL_MACOSX_APPLICATION_MM

#import <Cocoa/Cocoa.h>
#undef check // solve a confliction with Boost

#include <cassert>

@interface EbitenController : NSObject<NSApplicationDelegate> {
@private
  NSWindow* window_;
}

- (id)initWithWindow:(NSWindow*)window;
- (void)applicationDidFinishLaunching:(NSNotification*)aNotification;

@end

#ifndef EBITEN_WITHOUT_OBJC_IMPL
@implementation EbitenController

- (id)initWithWindow:(NSWindow*)window {
  self = [super init];
  if (self != nil) {
    self->window_ = window;
  }
  return self;
}

- (void)initMenu {
  NSString* processName = [[NSProcessInfo processInfo] processName];
  NSMenu* mainMenu = [[NSMenu alloc] init];
  NSMenu* appleMenu = [[NSMenu alloc] init];
  [appleMenu addItemWithTitle:[@"Quit " stringByAppendingString:processName]
                       action:@selector(performClose:)
                keyEquivalent:@"q"];
  {
    NSMenuItem* menuItem = [mainMenu insertItemWithTitle:@""
                                                  action:nil
                                           keyEquivalent:@""
                                                 atIndex:0];
    [mainMenu setSubmenu:appleMenu forItem:menuItem];
  }
  [NSApp setMainMenu: mainMenu];
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
  (void)aNotification;
  NSWindow* window = self->window_;
  assert(window);
  [window makeKeyAndOrderFront:nil];
  [self initMenu];
}

@end
#endif

static void
ebiten_kernels_detail_run_application(ebiten::graphics::view& view) {
  @autoreleasepool {
    NSWindow* window = [view.native_view() window];
    assert(window != nil);
    EbitenController* controller = [[EbitenController alloc] initWithWindow:window];
    NSApplication* app = [NSApplication sharedApplication];
    [app setDelegate:controller];
    [app finishLaunching];
    [app run];
  }
}

#endif
