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

#include "app.h"

#include <hal.h>

#include "common/sys/time.h"
#include "common/util/singleton.h"
#include "hwconf/hw.h"
#include "mc_interface.h"
#include "esc_host.h"
#include "timeout.h"

namespace voyager {

/* VESC Powertrain Control Module Host ****************************************/

class VescEscHost : public EscHost {
 public:
  VescEscHost(SerialDriver *serial_device,
      SerialConfig *serial_config)
          : EscHost(serial_device, serial_config) {}

 protected:
  virtual void ProcessExchangeState(
      const voyager_EscExchangeStateRequest& state) override {
    // TODO: Implement some scaling.
    mc_interface_set_current(0.75f + state.throttle_position);
    timeout_reset();
  }

  virtual void FillExchangeStateResponse(
      voyager_EscExchangeStateResponse *state) override {
    state->motor_rpm = mc_interface_get_rpm();
  }
};

/* PCM Host Thread ************************************************************/

static THD_WORKING_AREA(pcmHostThreadWorkingArea, 2048);

static THD_FUNCTION(pcmHostThread, arg) {
  Singleton<VescEscHost>::Instance()->Start();
}

/* Init/Config ****************************************************************/

extern "C" void app_uartcomm_start() {
  palSetPadMode(HW_SERIAL_TX_PORT, HW_SERIAL_TX_PIN,
      PAL_MODE_ALTERNATE(HW_SERIAL_GPIO_AF));
  palSetPadMode(HW_SERIAL_RX_PORT, HW_SERIAL_RX_PIN,
      PAL_MODE_ALTERNATE(HW_SERIAL_GPIO_AF));

  SerialConfig serial_config = {
    .speed = 115200,
    .cr1 = 0,
    .cr2 = 0,
    .cr3 = 0,
  };

  voyager::InitTime();
  Singleton<VescEscHost>::Init(
      &HW_SERIAL_DEV, &serial_config);

  chThdCreateStatic(pcmHostThreadWorkingArea, sizeof(pcmHostThreadWorkingArea),
      NORMALPRIO, pcmHostThread, NULL);
}

extern "C" void app_uartcomm_configure(uint32_t baudrate) {
  // This function has no implementation. Baud rate is hardcoded to match that
  // of the PCM.
}

}  // namespace voyager
