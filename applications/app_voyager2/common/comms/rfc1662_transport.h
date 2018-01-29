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

#ifndef VOYAGER_COMMON_COMMS_RFC1662_TRANSPORT_H_
#define VOYAGER_COMMON_COMMS_RFC1662_TRANSPORT_H_

#include <cstddef>
#include <cstdint>

#include "common/comms/crc16.h"
#include "common/util/non_copyable.h"

namespace voyager {

//! The default size for an Rfc1662Transport.
constexpr size_t kDefaultRfc1662TransportSize = 1024;

//! The delimiter byte to separate frames.
constexpr uint8_t kRfc1662Delimiter = 0x7e;

//! The escape byte to escape delimiters and escapes themselves.
constexpr uint8_t kRfc1662Escape = 0x7d;

/**
 * Return values for the Rfc1662Transport::SendFrame method.
 */
enum class SendFrameResult {
  //! The frame was sent successfully.
  Success,

  //! The requested frame is too large to send.
  FrameTooLarge,

  //! A link error occurred when sending the packet across the physical media.
  LinkError,
};

/**
 * Return values for the Rfc1662Transport::ReceiveFrame method.
 */
enum class ReceiveFrameResult {
  //! The frame was received successfully.
  Success,

  //! The received frame was malformed. This means that an invalid escape
  //! sequence or start byte was received.
  MalformedFrame,

  //! The received frame was too large.
  FrameTooLarge,

  //! The frame had a CRC mismatch.
  CrcMismatch,

  //! A link error occurred when reading the packet from the physical media.
  LinkError,

  //! A timeout occurred while receiving this frame.
  Timeout,
};

/**
 * Implements an abstract data transport using the RFC1662 protocol to encode
 * frames. This means that all written data is framed with a checksum, SoM+EoM
 * delimiting bytes and escape sequences for reserved characters.
 *
 * This transport is intended to be used in environments where a link could be
 * lossy and bytes dropped or bits flipped. A good use case for this transport
 * is for USART communications between systems.
 *
 * This transport is abstract such that the system is expected to supply the
 * LinkRead and LinkWrite method implementations. This allows easy porting to
 * different platforms such as POSIX, ChibiOS, libftdi, etc.
 */
template<size_t kBufferSize = kDefaultRfc1662TransportSize>
class Rfc1662Transport : public NonCopyable {
 public:
  /**
   * Constructs an Rfc1662Transport and sets up initial state.
   */
  Rfc1662Transport();

  /**
   * Sends the provided frame through the transport. The result indicates
   * success or the type of error that has occured.
   *
   * @param frame the frame to encode and transmit.
   * @param frame_len the size of the frame to send.
   * @return the result of the operation.
   */
  SendFrameResult SendFrame(const uint8_t *frame, size_t frame_len);

  /**
   * Receives a frame from the transport. The result indicates success or the
   * type of error that has occured.
   *
   * @param frame a pointer to a frame to populate with the receive frame.
   * @param frame_len a pointer to the size of the frame that was received.
   * @param timeout_us the maximum amount of time to read this frame in
   *        microseconds.
   * @return the result of the operation. If Success, the frame and frame_len
   *         pointers are assigned.
   */
  ReceiveFrameResult ReceiveFrame(const uint8_t **frame, size_t *frame_len,
      uint64_t timeout_us);

 protected:
  /**
   * A pure-virtual write method that takes a buffer to write and the size of
   * that buffer. This method will write the buffer and return the number of
   * bytes written or return zero if an error occurs. Valid implementations
   * including writing byte-by-byte, blocks or even the entire buffer all at
   * once.
   *
   * @param buffer the buffer to write.
   * @param length the size of the buffer to write.
   * @return the number of bytes written or zero on error.
   */
  virtual size_t LinkWrite(const uint8_t *buffer, size_t length) = 0;

  /**
   * A pure-virtual read function that takes a buffer to write into and the
   * size of that buffer. This method will read from a data source and return
   * the number of bytes read or zero if an error occurs. Valid implementations
   * include reading byte-by-byte or blocks at a time as long as the buffer is
   * not overflowed.
   *
   * @param buffer the buffer to read from.
   * @param length the size of the buffer to read into.
   * @param timeout_us the maximum amount of time that this read operation can
   *        block for.
   * @return the number of bytes read, SIZE_MAX on timeout and 0 on error.
   */
  virtual size_t LinkRead(uint8_t *buffer, size_t length,
      uint64_t timeout_us) = 0;

  /**
   * A pure-virtual method to obtain the current system time in microseconds.
   * This function must be monotonic and is used to implement protocol timeouts.
   *
   * @param The current system time in microseconds.
   */
  virtual uint64_t GetSystemTimeUs() = 0;

 private:
  //! Reserve space when receiving a packet to do an in-place decode without
  //! corrupting the CRC.
  static constexpr size_t kReceivePadding = sizeof(uint16_t);

  //! The buffer to read/write packets into.
  uint8_t buffer_[kBufferSize];

  //! The receive position to read at when decoding multiple frames back to
  //! back.
  size_t buffer_size_;

  //! The position in the buffer to read from.
  size_t read_pos_;

  /**
   * Encodes a frame into the class-private buffer.
   *
   * @param frmae the frame to encode.
   * @param frame_len the length of the frame to encode.
   * @param buffer_pos the final buffer position after the frame has been
   *        encoded.
   * @return the result of encoding the frame.
   */
  SendFrameResult EncodeFrame(const uint8_t *frame, size_t frame_len,
                              size_t *buffer_pos);

  /**
   * Writes a frame to the underlying link. This will invoke LinkWrite as many
   * times as necessary to flush the entire frame. If an error occurs, LinkWrite
   * will return 0 and LinkError will be returned from this function.
   *
   * @param buffer_pos the size of the buffer to write out.
   * @return Success if the entire frame is written, LinkError otherwise.
   */
  SendFrameResult WriteFrame(size_t buffer_pos);

  /**
   * Verifies that the contents of the buffer contain a valid CRC.
   *
   * @param frame_len the size of the frame currently stored in the buffer.
   * @return true if the frame is valid.
   */
  bool VerifyReceivedCrc(size_t frame_len);
};

}  // namespace voyager

#include "common/comms/rfc1662_transport_impl.h"

#endif  // VOYAGER_COMMON_COMMS_RFC1662_TRANSPORT_H_
