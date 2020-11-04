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

#include <arpa/inet.h>
#include <boost/crc.hpp>
#include <endian.h>

#include <algorithm>
#include <sstream>
#include <string>
#include <iostream>

#include "psen_scan_v2/angle_conversions.h"
#include "psen_scan_v2/start_request.h"
#include "psen_scan_v2/raw_processing.h"

namespace psen_scan_v2
{
static constexpr TenthOfDegree MASTER_RESOLUTION{ TenthOfDegree(2) };

start_request::Message::Message(const ScannerConfiguration& scanner_configuration)
  : host_ip_(scanner_configuration.hostIp())
  , host_udp_port_data_(scanner_configuration.hostUDPPortData())  // Write is deduced by the scanner
  , master_(scanner_configuration.scanRange(), MASTER_RESOLUTION)
{
  device_settings_master_.setDiagnosticsEnabled(scanner_configuration.diagnosticsEnabled());
}
}  // namespace psen_scan_v2
