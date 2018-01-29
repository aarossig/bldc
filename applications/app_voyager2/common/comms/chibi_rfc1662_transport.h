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

#ifndef VOYAGER_FIRMWARE_COMMON_COMMS_CHIBI_RFC1662_TRANSPORT_H_
#define VOYAGER_FIRMWARE_COMMON_COMMS_CHIBI_RFC1662_TRANSPORT_H_

#include <hal.h>

#include "common/comms/rfc1662_transport.h"

namespace voyager {

/**
 * Implements the Rfc1662Transport using a POSIX standard tty device.
 */
template<size_t kBufferSize = kDefaultRfc1662TransportSize>
class ChibiRfc1662Transport : public Rfc1662Transport<kBufferSize> {
 public:
  /**
   * Constructs the ChibiRfc1662Transport given a pointer to a SerialDriver.
   *
   * @param serial_driver the serial device to use for read/write operations.
   * @param serial_config the configuration to use for the serial device.
   */
  ChibiRfc1662Transport(SerialDriver *serial_driver,
                        SerialConfig *serial_config);

  /**
   * Stops the Chibi SerialDriver.
   */
  ~ChibiRfc1662Transport();

 protected:
  // Implementations of the LinkRead/LinkWrite methods of the Rfc1662Transport
  // class.
  virtual size_t LinkWrite(const uint8_t *buffer, size_t length) override;
  virtual size_t LinkRead(uint8_t *buffer, size_t length,
      uint64_t timeout_us) override;
  virtual uint64_t GetSystemTimeUs() override;

 private:
  //! The serial device to read from/write to.
  SerialDriver *serial_driver_;
};

}  // namespace voyager

#include "common/comms/chibi_rfc1662_transport_impl.h"

#endif  // VOYAGER_FIRMWARE_COMMON_COMMS_CHIBI_RFC1662_TRANSPORT_H_
