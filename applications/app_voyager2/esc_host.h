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

#ifndef VOYAGER_FIRMWARE_ESC_ESC_HOST_H_
#define VOYAGER_FIRMWARE_ESC_ESC_HOST_H_

#include <cstdint>
#include <hal.h>

#include "common/comms/chibi_rfc1662_transport.h"
#include "common/comms/esc_interface.pb.h"
#include "common/comms/pcm_common.h"
#include "common/util/non_copyable.h"

namespace voyager {

class EscHost : public NonCopyable {
 public:
  /**
   * Sets up a powertrain contorl manager host.
   *
   * @param serial_device a serial device to listen to commands on.
   * @param serial_config the configuration for this serial device.
   */
  EscHost(SerialDriver *serial_device, SerialConfig *serial_config);

  /**
   * Starts the powertrain control manager host. This is a blocking call that
   * does not return. It is expected to block for the lifespan of the system
   * after initialization is completed. It will receive messages from the
   * powertrain control manager and respond to them.
   */
  void Start();

 protected:
  /**
   * Handles an incoming state exchange.
   *
   * @param state The state to process and update local configuration.
   */
  virtual void ProcessExchangeState(
      const voyager_EscExchangeStateRequest& state) = 0;

  /**
   * Fills an ESC state exchange with current system state.
   *
   * @param state The state to fill in to be sent to the client.
   */
  virtual void FillExchangeStateResponse(
      voyager_EscExchangeStateResponse *state) = 0;

 private:
  //! The timeout duration for receiving a command from the PCM.
  constexpr static uint64_t kRequestTimeoutUs = 100000;

  //! The underlying transport layer to send/receive frames from.
  ChibiRfc1662Transport<> transport_;

  //! The buffer to encode response messages into.
  uint8_t response_buffer_[kMaxPcmMessageSize];

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

  /**
   * Serializes and sends the current response in the response_buffer. This
   * function must only be called when a valid response has been populated.
   */
  void SendResponse();
};

}  // namespace voyager

#endif  // VOYAGER_FIRMWARE_ESC_ESC_HOST_H_
