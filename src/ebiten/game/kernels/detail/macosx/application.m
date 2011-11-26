#import <Cocoa/Cocoa.h>
#undef check // conflicts with boost
#include <cassert>

@interface EbitenController : NSObject<NSApplicationDelegate> {
@private
  NSWindow* window_;
}

- (id)initWithWindow:(NSWindow*)window;
- (void)applicationDidFinishLaunching:(NSNotification*)aNotification;

@end

@implementation EbitenController

- (id)initWithWindow:(NSWindow*)window {
  self = [super init];
  if (self != nil) {
    self->window_ = window;
  }
  return self;
}

- (void)initMenu {
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  // TODO: add the application's name
  NSMenu* mainMenu = [[NSMenu alloc] initWithTitle:@""];
  NSMenu* appleMenu = [[NSMenu alloc] initWithTitle:@""];
  [appleMenu addItemWithTitle:@"Quit" 
                       action:@selector(terminate:)
                keyEquivalent:@"q"];
  {
    NSMenuItem* menuItem = [mainMenu insertItemWithTitle:@""
                                                  action:nil
                                           keyEquivalent:@""
                                                 atIndex:0];
    [mainMenu setSubmenu:appleMenu forItem:menuItem];
  }
  [NSApp setMainMenu: mainMenu];
  [mainMenu release];
  [appleMenu release];
  [pool release];
}

- (void)applicationDidFinishLaunching:(NSNotification*)aNotification {
  (void)aNotification;
  NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
  NSWindow* window = self->window_;
  assert(window);
  [window makeKeyAndOrderFront:nil];
  [self initMenu];
  [pool release];
}

@end
