/*
 * Copyright 2017 Andrew Rossignol andrew.rossignol@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "common/sys/time.h"

#include <hal.h>

namespace voyager {

//! The number of times that the 32-bit timer has rolled over.
uint32_t gRolloverCount = 0;

/**
 * Invoked when the timestamp timer overflows.
 */
static void onTimerOverflow(GPTDriver *generalPurposeTimer) {
  gRolloverCount++;
}

void InitTime() {
  // Configures a 32-bit timer at 1MHz which will roll over roughly every 1.2
  // hours. The rollover events are also counted which gives this clock a
  // rollover time of ~500 000 years. This ought to be enough for everyone.
  static const GPTConfig timerConfig = {
  	1000000, // 1MHz time keeping clock
  	onTimerOverflow, // Callback to invoke on overflow.
  	0,
    0,
  };

	// Initialize and start the timer used to timestamp events.
	gptStart(&GPTD5, &timerConfig);
	gptStartContinuous(&GPTD5, UINT32_MAX);
}

uint64_t GetMonotonicTime() {
  uint32_t timerTicks = gptGetCounterX(&GPTD5);
  return (static_cast<uint64_t>(gRolloverCount) << 32) | timerTicks;
}

}  // namespace voyager
