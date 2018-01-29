/*
 * Copyright 2017 Andrew Rossignol andrew.rossignol@gmail.com
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef VOYAGER_FIRMWARE_COMMON_COMMS_CHIBI_RFC1662_TRANSPORT_IMPL_H_
#define VOYAGER_FIRMWARE_COMMON_COMMS_CHIBI_RFC1662_TRANSPORT_IMPL_H_

#include "common/comms/chibi_rfc1662_transport.h"

#include "common/sys/time.h"

namespace voyager {

template<size_t kBufferSize>
ChibiRfc1662Transport<kBufferSize>::ChibiRfc1662Transport(
    SerialDriver *serial_driver, SerialConfig *serial_config)
        : serial_driver_(serial_driver) {
  sdStart(serial_driver, serial_config);    
}

template<size_t kBufferSize>
ChibiRfc1662Transport<kBufferSize>::~ChibiRfc1662Transport() {
  sdStop(serial_driver_);
}

template<size_t kBufferSize>
size_t ChibiRfc1662Transport<kBufferSize>::LinkWrite(const uint8_t *buffer,
                                                     size_t length) {
  return sdWrite(serial_driver_, buffer, length);
}

template<size_t kBufferSize>
size_t ChibiRfc1662Transport<kBufferSize>::LinkRead(uint8_t *buffer,
                                                    size_t length,
                                                    uint64_t timeout_us) {
  size_t bytes_read = sdReadTimeout(serial_driver_, buffer, 1,
      US2ST(timeout_us));
  if (bytes_read == 0) {
    // Use SIZE_MAX to indicate timeout.
    bytes_read = SIZE_MAX;
  }

  return bytes_read;
}

template<size_t kBufferSize>
uint64_t ChibiRfc1662Transport<kBufferSize>::GetSystemTimeUs() {
  return GetMonotonicTime();
}

}  // namespace voyager

#endif  // VOYAGER_FIRMWARE_COMMON_COMMS_CHIBI_RFC1662_TRANSPORT_IMPL_H_
