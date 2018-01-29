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

#ifndef VOYAGER_COMMON_COMMS_RFC1662_TRANSPORT_IMPL_H_
#define VOYAGER_COMMON_COMMS_RFC1662_TRANSPORT_IMPL_H_

#include "common/comms/rfc1662_transport.h"

#include <cinttypes>
#include <cstring>

namespace voyager {

/**
 * Inserts a byte into the provided buffer and performs escaping as necessary.
 * If the provided byte does not fit into the buffer (with escaping if needed)
 * the function returns false.
 *
 * @param byte the byte to insert into the buffer
 * @param dest a pointer to a buffer to insert the byte into
 * @param dest_pos the position in the buffer to insert at
 * @param dest_len the size of the provided buffer
 * @return true if the byte was inserted into the buffer
 */
bool InsertEscapedByte(uint8_t byte, uint8_t *dest, size_t *dest_pos,
                       size_t dest_len);

template<size_t kBufferSize>
Rfc1662Transport<kBufferSize>::Rfc1662Transport()
    : buffer_size_(kReceivePadding),
      read_pos_(kReceivePadding) {}

template<size_t kBufferSize>
SendFrameResult Rfc1662Transport<kBufferSize>::SendFrame(const uint8_t *frame,
                                                         size_t frame_len) {
  size_t buffer_pos = 0;
  SendFrameResult result = EncodeFrame(frame, frame_len, &buffer_pos);
  if (result == SendFrameResult::Success) {
    result = WriteFrame(buffer_pos);
  }

  return result;
}

template<size_t kBufferSize>
ReceiveFrameResult Rfc1662Transport<kBufferSize>::ReceiveFrame(
    const uint8_t **frame, size_t *frame_len, uint64_t timeout_us) {
  enum class ReceiveState {
    Reset,
    InFrame,
    InEscape,
  };

  size_t write_pos = 0;
  uint64_t receive_deadline = GetSystemTimeUs() + timeout_us;
  ReceiveState receive_state = ReceiveState::Reset;
  ReceiveFrameResult result = ReceiveFrameResult::Success;

  while (result == ReceiveFrameResult::Success) {
    if (read_pos_ >= buffer_size_) {
      size_t read_amount = kBufferSize - buffer_size_;
      if (read_amount > 0) {
        uint64_t current_time = GetSystemTimeUs();
        if (receive_deadline > current_time) {
          uint64_t timeout_us = receive_deadline - current_time;
          size_t bytes_read = LinkRead(&buffer_[buffer_size_], read_amount,
              timeout_us);
          if (bytes_read == SIZE_MAX) {
            result = ReceiveFrameResult::Timeout;
          } else if (bytes_read == 0) {
            result = ReceiveFrameResult::LinkError;
          } else {
            receive_deadline = current_time + timeout_us;
            buffer_size_ += bytes_read;
          }
        } else {
          result = ReceiveFrameResult::Timeout;
        }
      } else {
        buffer_size_ = kReceivePadding;
        read_pos_ = kReceivePadding;
        result = ReceiveFrameResult::FrameTooLarge;
      }
    }

    if (read_pos_ < buffer_size_) {
      uint8_t byte = buffer_[read_pos_++];
      if (receive_state == ReceiveState::Reset) {
        if (byte == kRfc1662Delimiter) {
          receive_state = ReceiveState::InFrame;
        } else {
          result = ReceiveFrameResult::MalformedFrame;
        }
      } else if (receive_state == ReceiveState::InFrame) {
        if (byte == kRfc1662Delimiter) {
          if (write_pos >= sizeof(uint16_t)) {
            break; // A complete frame has been received.
          } else if (write_pos > 0 && write_pos < sizeof(uint16_t)) {
            result = ReceiveFrameResult::MalformedFrame;
          }
        } else if (byte == kRfc1662Escape) {
          receive_state = ReceiveState::InEscape;
        } else {
          buffer_[write_pos++] = byte;
        }
      } else if (receive_state == ReceiveState::InEscape) {
        if (byte == kRfc1662Delimiter || byte == kRfc1662Escape) {
          buffer_[write_pos++] = byte;
          receive_state = ReceiveState::InFrame;
        } else {
          result = ReceiveFrameResult::MalformedFrame;
        }
      }
    }
  }

  // Verify the CRC.
  if (result == ReceiveFrameResult::Success) {
    if (VerifyReceivedCrc(write_pos)) {
      *frame = buffer_;
      *frame_len = write_pos - 2;
    } else {
      result = ReceiveFrameResult::CrcMismatch;
      write_pos = kReceivePadding;
    }
  } else {
    write_pos = kReceivePadding;
  }

  // Move received contents after the currently received frame.
  if (buffer_size_ >= read_pos_) {
    size_t remaining_bytes = buffer_size_ - read_pos_;
    if (remaining_bytes > 0) {
      memcpy(&buffer_[write_pos], &buffer_[read_pos_], remaining_bytes);
    }

    buffer_size_ = write_pos + remaining_bytes;
    read_pos_ = write_pos;
  }

  return result;
}

template<size_t kBufferSize>
SendFrameResult Rfc1662Transport<kBufferSize>::EncodeFrame(const uint8_t *frame,
                                                           size_t frame_len,
                                                           size_t *buffer_pos) {
  size_t pos = 0;
  SendFrameResult result = SendFrameResult::FrameTooLarge;

  // Check that there is at least space for the checksum (minimum 2 bytes),
  // start and end of message bytes in addition to a message that requires no
  // escape characters. This is an attempt to bail early if it is known up
  // front that the destination buffer cannot possibly hold the source message.
  if ((frame_len + 4) <= kBufferSize) {
    // Insert the start of message byte.
    buffer_[pos++] = kRfc1662Delimiter;

    // Escape the source message.
    result = SendFrameResult::Success;
    for (size_t i = 0; i < frame_len; i++) {
      if (!InsertEscapedByte(frame[i], buffer_, &pos, kBufferSize)) {
        result = SendFrameResult::FrameTooLarge;
        break;
      }
    }

    // Insert the CRC and end delimieter if successful.
    if (result == SendFrameResult::Success) {
      uint16_t crc = GenerateCrc16(frame, frame_len);
      if (!InsertEscapedByte((crc >> 8), buffer_, &pos, kBufferSize)
          || !InsertEscapedByte((crc & 0xff), buffer_, &pos, kBufferSize)) {
        result = SendFrameResult::FrameTooLarge;
      }

      if (result == SendFrameResult::Success && pos < kBufferSize) {
        buffer_[pos++] = kRfc1662Delimiter;
      } else {
        result = SendFrameResult::FrameTooLarge;
      }
    }
  }

  *buffer_pos = pos;
  return result;
}

template<size_t kBufferSize>
SendFrameResult Rfc1662Transport<kBufferSize>::WriteFrame(size_t buffer_pos) {
  SendFrameResult result = SendFrameResult::Success;
  size_t total_bytes_written = 0;
  while (total_bytes_written < buffer_pos) {
    size_t bytes_written = LinkWrite(&buffer_[total_bytes_written],
                                     buffer_pos - total_bytes_written);
    if (bytes_written == 0) {
      result = SendFrameResult::LinkError;
      break;
    } else {
      total_bytes_written += bytes_written;
    }
  }

  return result;
}

template<size_t kBufferSize>
bool Rfc1662Transport<kBufferSize>::VerifyReceivedCrc(size_t frame_len) {
  uint16_t crc = GenerateCrc16(buffer_, frame_len - 2);
  return ((crc >> 8) == buffer_[frame_len - 2]
      && (crc & 0xff) == buffer_[frame_len - 1]);
}

}  // namespace voyager

#endif  // VOYAGER_COMMON_COMMS_RFC1662_TRANSPORT_IMPL_H_
