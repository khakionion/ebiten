#include "ebiten/ebiten.hpp"
#include "ebiten/frames/frame.hpp"
#include "ebiten/kernels/application.hpp"

#include "ebiten_script/squirrel/game.hpp"

int
main() {
  try {
    NSBundle* bundle = [NSBundle mainBundle];
    NSString* ns_path = [bundle pathForResource:@"test.nut" ofType:nil];
    std::string path;
    if (ns_path != nil) {
      path = [ns_path UTF8String];
    } else {
      path = "test.nut";
    }
    ebiten_script::squirrel::game game(path);
    auto game_update = std::bind(&ebiten_script::squirrel::game::update,
                                 &game,
                                 std::placeholders::_1);
    auto game_draw   = std::bind(&ebiten_script::squirrel::game::draw,
                                 &game,
                                 std::placeholders::_1,
                                 std::placeholders::_2);
    ebiten::frames::frame frame(640, 480);
    ebiten::kernels::kernel kernel(game_update,
                                   game_draw,
                                   320, 240, 2, 60,
                                   frame.native_view());
    ebiten::kernels::application::run(frame.native_frame());
  } catch (std::runtime_error const& err) {
    std::cerr << err.what() << std::endl;
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
