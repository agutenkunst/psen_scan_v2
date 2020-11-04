// Copyright (c) 2020 Pilz GmbH & Co. KG
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.
#ifndef PSEN_SCAN_V2_STOP_REQUEST_SERIALIZATION_H
#define PSEN_SCAN_V2_STOP_REQUEST_SERIALIZATION_H

#include "psen_scan_v2/stop_request.h"

namespace psen_scan_v2
{
namespace stop_request
{
static constexpr std::size_t SIZE{ 20 };
static constexpr std::size_t NUM_RESERVED_FIELDS{ 12 };
const std::array<uint8_t, NUM_RESERVED_FIELDS> RESERVED{};
const uint32_t OPCODE{ htole32(0x36) };

DynamicSizeRawData serialize(const stop_request::Message& stop_request);
}  // namespace stop_request
}  // namespace psen_scan_v2
#endif