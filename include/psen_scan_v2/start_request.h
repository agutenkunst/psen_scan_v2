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
#ifndef PSEN_SCAN_V2_START_REQUEST_H
#define PSEN_SCAN_V2_START_REQUEST_H

#include <array>
#include <cstdint>
#include <string>

#include "psen_scan_v2/scanner_configuration.h"
#include "psen_scan_v2/raw_scanner_data.h"
#include "psen_scan_v2/scan_range.h"
#include "psen_scan_v2/scanner_ids.h"

namespace psen_scan_v2
{
namespace start_request
{
/**
 * @brief Higher level data type representing a scanner start request.
 *
 * @note Unless otherwise indicated the byte order is little endian.
 *
 */
class Message
{
public:
  /**
   * @brief Constructor.
   *
   * @param scanner_configuration Specifies the required scanner configuration.
   */
  Message(const ScannerConfiguration& scanner_configuration);

  friend psen_scan_v2::DynamicSizeRawData
  psen_scan_v2::start_request::serialize(const psen_scan_v2::start_request::Message& start_request,
                                         const uint32_t& seq_number);
  friend psen_scan_v2::DynamicSizeRawData
  psen_scan_v2::start_request::serialize(const psen_scan_v2::start_request::Message& start_request);

private:
  uint32_t host_ip_;
  uint16_t host_udp_port_data_;

  class LaserScanSettings
  {
  public:
    LaserScanSettings() = default;

    LaserScanSettings(const DefaultScanRange& scan_range, const TenthOfDegree resolution)
      : scan_range_(scan_range), resolution_(resolution)
    {
    }

  public:
    const DefaultScanRange& getScanRange() const
    {
      return scan_range_;
    };

    TenthOfDegree getResolution() const
    {
      return resolution_;
    };

  private:
    const DefaultScanRange scan_range_{};
    TenthOfDegree resolution_{ 0 };
  };
  class DeviceSettings
  {
  public:
    DeviceSettings(ScannerId id, bool diagnostics_enabled) : id_(id), diagnostics_enabled_(diagnostics_enabled)
    {
    }

  public:
    bool getDiagnosticsEnabled() const
    {
      return diagnostics_enabled_;
    };

    void setDiagnosticsEnabled(bool value)
    {
      diagnostics_enabled_ = value;
    };

  private:
    ScannerId id_;
    bool diagnostics_enabled_;
  };

private:
  DeviceSettings device_settings_master_{ DeviceSettings(ScannerId::master, true) };
  std::array<DeviceSettings, 3> device_settings_slaves_{ DeviceSettings(ScannerId::slave0, false),
                                                         DeviceSettings(ScannerId::slave1, false),
                                                         DeviceSettings(ScannerId::slave2, false) };

  LaserScanSettings master_;
  std::array<LaserScanSettings, 3> slaves_;
};

}  // namespace start_request
}  // namespace psen_scan_v2

#endif  // PSEN_SCAN_V2_START_REQUEST_H
