#ifndef EBITEN_TIMERS_DETAIL_MACOSX_TIMER_HPP
#define EBITEN_TIMERS_DETAIL_MACOSX_TIMER_HPP

#include "ebiten/noncopyable.hpp"
#include <mach/mach_time.h>
#include <cassert>
#include <cstddef>

namespace ebiten {
namespace timers {
namespace detail {

class timer : private noncopyable {
public:
  static uint64_t
  now_nsec() {
    mach_timebase_info_data_t timebase_info;
    mach_timebase_info(&timebase_info);
    uint64_t const now = mach_absolute_time();
    return now * timebase_info.numer / timebase_info.denom;
  }
private:
  timer() {
  }
};

}
}
}

#endif
