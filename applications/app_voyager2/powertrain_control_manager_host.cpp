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

#include "powertrain_control_manager_host.h"

namespace voyager {

PowertrainControlManagerHost::PowertrainControlManagerHost(
    SerialDriver *serial_device, SerialConfig *serial_config)
        : transport_(serial_device, serial_config) {}

void PowertrainControlManagerHost::Start() {
  while (1) {
    const uint8_t *frame = nullptr;
    size_t frame_len = 0;

    auto result = transport_.ReceiveFrame(&frame, &frame_len,
        kRequestTimeoutUs);
    if (result == ReceiveFrameResult::Success) {
      HandleFrame(frame, frame_len);
    } else {
      // TODO: Handle receive failures. The best idea would be to idle the
      // motors, but some care needs to be taken. If comms are intermittent,
      // we don't want the bike janking in and out of control.
    }
  }
}

void PowertrainControlManagerHost::HandleFrame(const uint8_t *frame,
    size_t frame_len) {
  pb_istream_t stream = pb_istream_from_buffer(frame, frame_len);
  if (!pb_decode(&stream, voyager_EscRequest_fields, &request_)) {
    // TODO: Handle decode failures.
  } else {
    switch (request_.which_request) {
      case voyager_EscRequest_esc_exchange_state_request_tag:
        HandleStateExchange(request_.esc_exchange_state_request);
        break;
      default:
        LOGE("Received invalid request");
        break;
    }
  }
}

void PowertrainControlManagerHost::HandleStateExchange(
    const voyager_EscExchangeStateRequest& state) {

}

}  // namespace voyager
