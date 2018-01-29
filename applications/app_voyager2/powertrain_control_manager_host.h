/*
 * Copyright 2018 Andrew Rossignol andrew.rossignol@gmail.com
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

#ifndef VOYAGER_FIRMWARE_ESC_POWERTRAIN_CONTROL_MANAGER_HOST_H_
#define VOYAGER_FIRMWARE_ESC_POWERTRAIN_CONTROL_MANAGER_HOST_H_

#include <cstdint>
#include <hal.h>

#include "common/comms/chibi_rfc1662_transport.h"
#include "common/comms/esc_interface.pb.h"
#include "common/util/non_copyable.h"

namespace voyager {

class PowertrainControlManagerHost : public NonCopyable {
 public:
  /**
   * Sets up a powertrain contorl manager host.
   *
   * @param serial_device a serial device to listen to commands on.
   * @param serial_config the configuration for this serial device.
   */
  PowertrainControlManagerHost(SerialDriver *serial_device,
      SerialConfig *serial_config);

  /**
   * Starts the powertrain control manager host. This is a blocking call that
   * does not return. It is expected to block for the lifespan of the system
   * after initialization is completed. It will receive messages from the
   * powertrain control manager and respond to them.
   */
  void Start();

 private:
  //! The timeout duration for receiving a command from the PCM.
  constexpr static uint64_t kRequestTimeoutUs = 100000;

  //! The underlying transport layer to send/receive frames from.
  ChibiRfc1662Transport<> transport_;

  //! The current request made by the PCM.
  voyager_EscRequest request_;

  //! The current response made to the PCM.
  voyager_EscResponse response_;

  /**
   * Handles an incoming frame to be decoded by nanopb and a response formatted
   * if necessary.
   */
  void HandleFrame(const uint8_t *frame, size_t frame_len);

  /**
   * Handles the state exchange between the PCM and ESC.
   *
   * @param state The state to assign to the ESC.
   */
  void HandleStateExchange(const voyager_EscExchangeStateRequest& state);
};

}  // namespace voyager

#endif  // VOYAGER_FIRMWARE_ESC_POWERTRAIN_CONTROL_MANAGER_HOST_H_
