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
#include "powertrain_control_manager_host.h"

/* PCM Host Thread ************************************************************/

static THD_WORKING_AREA(pcmHostThreadWorkingArea, 2048);

static THD_FUNCTION(pcmHostThread, arg) {
  voyager::Singleton<voyager::PowertrainControlManagerHost>::Instance()
      ->Start();
}

/* Init/Config ****************************************************************/

extern "C" void app_uartcomm_start() {
  palSetPadMode(HW_SERIAL_TX_PORT, HW_SERIAL_TX_PIN,
      PAL_MODE_ALTERNATE(HW_UART_GPIO_AF)
          | PAL_STM32_OSPEED_HIGHEST
          | PAL_STM32_PUDR_PULLUP);
  palSetPadMode(HW_SERIAL_RX_PORT, HW_SERIAL_RX_PIN,
      PAL_MODE_ALTERNATE(HW_UART_GPIO_AF)
          | PAL_STM32_OSPEED_HIGHEST
          | PAL_STM32_PUDR_PULLUP);

  SerialConfig serial_config = {
    .speed = 115200,
    .cr1 = 0,
    .cr2 = 0,
    .cr3 = 0,
  };

  voyager::InitTime();
  voyager::Singleton<voyager::PowertrainControlManagerHost>::Init(
      &HW_SERIAL_DEV, &serial_config);

  chThdCreateStatic(pcmHostThreadWorkingArea, sizeof(pcmHostThreadWorkingArea),
      NORMALPRIO, pcmHostThread, NULL);
}

extern "C" void app_uartcomm_configure(uint32_t baudrate) {
  // This function has no implementation. Baud rate is hardcoded to match that
  // of the PCM.
}
