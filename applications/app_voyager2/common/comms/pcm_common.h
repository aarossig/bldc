/*
 * Copyright 2018 Andrew Rossignol andrew.rossignol@gmail.com
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

#ifndef VOYAGER_FIRMWARE_COMMON_COMMS_PCM_COMMON_H_
#define VOYAGER_FIRMWARE_COMMON_COMMS_PCM_COMMON_H_

/**
 * @file Common code that is shared between the pcm and esc modules.
 */

namespace voyager {

//! The maximum size of an encoded ESC message.
constexpr size_t kMaxPcmMessageSize = 512;

}  // namespace voyager

#endif  // VOYAGER_FIRMWARE_COMMON_COMMS_PCM_COMMON_H_
