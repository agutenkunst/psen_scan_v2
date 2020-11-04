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

#ifndef PSEN_SCAN_V2_SCANNER_REPLY_MSG_H
#define PSEN_SCAN_V2_SCANNER_REPLY_MSG_H

#include <cstdint>
#include <cassert>
#include <array>
#include <sstream>

#include <boost/crc.hpp>

#include "psen_scan_v2/raw_scanner_data.h"
#include "psen_scan_v2/raw_processing.h"

namespace psen_scan_v2
{
namespace scanner_reply
{
/**
 * @brief Defines the possible types of reply messages which can be received from the scanner.
 */
enum class Type : uint32_t
{
  unknown = 0,
  start = 0x35,
  stop = 0x36,
};

/**
 * @brief Defines the operation result from the scanner.
 */
enum class OperationResult : uint32_t
{
  accepted = 0x00,
  refused = 0xEB,
  unknown = 0xFF
};

/**
 * @brief Higher level data type representing a reply message from the scanner.
 */
class Message
{
public:
  Message(Type& type, OperationResult& result);

  Type type() const;
  OperationResult result() const;

private:
  const Type type_;
  const OperationResult result_;
};

inline Type Message::type() const
{
  return type_;
}

inline OperationResult Message::result() const
{
  return result_;
}
inline Message::Message(Type& type, OperationResult& result) : type_(type), result_(result)
{
}
}  // namespace scanner_reply
}  // namespace psen_scan_v2
#endif  // PSEN_SCAN_V2_SCANNER_REPLY_MSG_H
