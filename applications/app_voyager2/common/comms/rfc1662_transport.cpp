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

#include "common/comms/rfc1662_transport.h"

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
                       size_t dest_len) {
  bool result = false;
  if (*dest_pos < dest_len) {
    if (byte == kRfc1662Delimiter || byte == kRfc1662Escape) {
      dest[(*dest_pos)++] = kRfc1662Escape;
    }

    if (*dest_pos < dest_len) {
      dest[(*dest_pos)++] = byte;
      result = true;
    }
  }

  return result;
}

}  // namespace voyager
