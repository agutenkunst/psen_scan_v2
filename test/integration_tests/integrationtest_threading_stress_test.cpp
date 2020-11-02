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

#include <future>
#include <thread>
#include <chrono>
#include <functional>

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <pilz_testutils/mock_appender.h>
#include <pilz_testutils/logger_mock.h>

#include <rosconsole_bridge/bridge.h>
REGISTER_ROSCONSOLE_BRIDGE;

// Test frameworks
#include "psen_scan_v2/async_barrier.h"
#include "psen_scan_v2/mock_udp_server.h"
#include "psen_scan_v2/udp_frame_dumps.h"
#include "psen_scan_v2/raw_data_array_conversion.h"
#include "psen_scan_v2/logging.h"

// Software under testing
#include "psen_scan_v2/scanner_configuration.h"
#include "psen_scan_v2/scanner_v2.h"
#include "psen_scan_v2/start_request.h"
#include "psen_scan_v2/scanner_reply_msg.h"
#include "psen_scan_v2/scan_range.h"
#include "psen_scan_v2/diagnostics.h"

namespace psen_scan_v2_test
{
using namespace psen_scan_v2;

static const std::string SCANNER_IP_ADDRESS{ "127.0.0.1" };
static const std::string HOST_IP_ADDRESS{ "127.0.0.1" };

static constexpr DefaultScanRange SCAN_RANGE{ TenthOfDegree(0), TenthOfDegree(1) };

static constexpr uint32_t DEFAULT_SEQ_NUMBER{ 0u };
static constexpr bool DIAGNOSTICS_ENABLED{ false };

using std::placeholders::_1;
using std::placeholders::_2;

using namespace ::testing;
using namespace std::chrono_literals;
using namespace pilz_testutils;

struct PortHolder
{
  PortHolder& operator++()
  {
    data_port_host = (data_port_host + 1) % MAX_DATA_PORT_HOST;
    if (data_port_host == 0)
    {
      data_port_host = MIN_DATA_PORT_HOST;
    }

    control_port_host = (control_port_host + 1) % MAX_CONTROL_PORT_HOST;
    if (control_port_host == 0)
    {
      control_port_host = MIN_CONTROL_PORT_HOST;
    }

    control_port_scanner = (control_port_scanner + 1) % MAX_CONTROL_PORT_SCANNER;
    if (control_port_scanner == 0)
    {
      control_port_scanner = MIN_CONTROL_PORT_HOST;
    }

    data_port_scanner = (data_port_scanner + 1) % MAX_DATA_PORT_SCANNER;
    if (data_port_scanner == 0)
    {
      data_port_scanner = MIN_CONTROL_PORT_HOST;
    }

    return *this;
  }

  void printPorts() const
  {
    std::cout << "Host ports:\n"
              << "- data port = " << data_port_host << "\n"
              << "- control port = " << control_port_host << "\n"
              << "Scanner ports:\n"
              << "- data port = " << control_port_scanner << "\n"
              << "- control port = " << data_port_scanner << "\n"
              << std::endl;
  }

  const int MIN_DATA_PORT_HOST{ 45000 };
  const int MAX_DATA_PORT_HOST{ 46000 };

  const int MIN_CONTROL_PORT_HOST{ 57000 };
  const int MAX_CONTROL_PORT_HOST{ 58000 };

  const unsigned short MIN_CONTROL_PORT_SCANNER{ 3000u };
  const unsigned short MAX_CONTROL_PORT_SCANNER{ 4000u };

  const unsigned short MIN_DATA_PORT_SCANNER{ 7000u };
  const unsigned short MAX_DATA_PORT_SCANNER{ 8000u };

  int data_port_host{ MIN_DATA_PORT_HOST };
  int control_port_host{ MIN_CONTROL_PORT_HOST };

  unsigned short control_port_scanner{ MIN_CONTROL_PORT_SCANNER };
  unsigned short data_port_scanner{ MIN_DATA_PORT_SCANNER };
};

static PortHolder GLOBAL_PORT_HOLDER;

ACTION_P(OpenBarrier, barrier)
{
  barrier->release();
}

class UserCallbacks
{
public:
  MOCK_METHOD1(LaserScanCallback, void(const LaserScan&));
};

class ScannerMock
{
public:
  ScannerMock(const PortHolder& port_holder)
    : control_msg_receiver_(
          udp::endpoint(boost::asio::ip::address_v4::from_string(HOST_IP_ADDRESS), port_holder.control_port_host))
    , monitoring_frame_receiver_(
          udp::endpoint(boost::asio::ip::address_v4::from_string(HOST_IP_ADDRESS), port_holder.data_port_host))
    , control_server_(port_holder.control_port_scanner, std::bind(&ScannerMock::receiveControlMsg, this, _1, _2))
    , data_server_(port_holder.data_port_scanner, std::bind(&ScannerMock::receiveDataMsg, this, _1, _2))
  {
  }

public:
  MOCK_METHOD2(receiveControlMsg, void(const udp::endpoint&, const psen_scan_v2::DynamicSizeRawData&));
  MOCK_METHOD2(receiveDataMsg, void(const udp::endpoint&, const psen_scan_v2::DynamicSizeRawData&));

public:
  void startListeningForControlMsg();
  void startContinuousListeningForControlMsg();

public:
  void sendStartReply();
  void sendStopReply();
  void sendMonitoringFrame(const monitoring_frame::Message& msg);
  void sendEmptyMonitoringFrame();

private:
  void sendReply(const uint32_t reply_type);

private:
  const udp::endpoint control_msg_receiver_;
  const udp::endpoint monitoring_frame_receiver_;

  MockUDPServer control_server_;
  MockUDPServer data_server_;
};

class ScannerAPIThreadingTests : public testing::Test
{
protected:
  void SetUp() override;

protected:
  const PortHolder port_holder_{ ++GLOBAL_PORT_HOLDER };

  ScannerConfiguration config_{
    HOST_IP_ADDRESS, port_holder_.data_port_host, port_holder_.control_port_host, SCANNER_IP_ADDRESS,
    SCAN_RANGE,      DIAGNOSTICS_ENABLED
  };
};

void ScannerAPIThreadingTests::SetUp()
{
  port_holder_.printPorts();
}

void ScannerMock::startListeningForControlMsg()
{
  control_server_.asyncReceive();
}

void ScannerMock::startContinuousListeningForControlMsg()
{
  control_server_.asyncReceive(MockUDPServer::ReceiveMode::continuous);
}

void ScannerMock::sendReply(const uint32_t reply_type)
{
  const ScannerReplyMsg msg(reply_type, 0x00);
  control_server_.asyncSend<REPLY_MSG_FROM_SCANNER_SIZE>(control_msg_receiver_, msg.serialize());
}

void ScannerMock::sendStartReply()
{
  std::cout << "ScannerMock: Send start reply..." << std::endl;
  sendReply(getOpCodeValue(ScannerReplyMsgType::Start));
}

void ScannerMock::sendStopReply()
{
  sendReply(getOpCodeValue(ScannerReplyMsgType::Stop));
}

void ScannerMock::sendMonitoringFrame(const monitoring_frame::Message& msg)
{
  std::cout << "ScannerMock: Send monitoring frame..." << std::endl;
  DynamicSizeRawData dynamic_raw_scan = serialize(msg);
  MaxSizeRawData max_size_raw_data = convertToMaxSizeRawData(dynamic_raw_scan);

  data_server_.asyncSend<max_size_raw_data.size()>(monitoring_frame_receiver_, max_size_raw_data);
}

void ScannerMock::sendEmptyMonitoringFrame()
{
  const psen_scan_v2::FixedSizeRawData<0> data;
  data_server_.asyncSend<data.size()>(monitoring_frame_receiver_, data);
}

TEST_F(ScannerAPIThreadingTests, shouldThrowWhenStartIsCalledTwice)
{
  NiceMock<ScannerMock> scanner_mock{ port_holder_ };
  UserCallbacks cb;
  ScannerV2 scanner(config_,
                    std::bind(&UserCallbacks::LaserScanCallback, &cb, std::placeholders::_1),
                    port_holder_.data_port_scanner,
                    port_holder_.control_port_scanner);

  Barrier start_req_received_barrier;
  ON_CALL(scanner_mock, receiveControlMsg(_, StartRequest(config_, DEFAULT_SEQ_NUMBER).serialize()))
      .WillByDefault(OpenBarrier(&start_req_received_barrier));

  scanner_mock.startListeningForControlMsg();
  const auto start_future = scanner.start();
  EXPECT_TRUE(start_future.valid()) << "First future should be valid";
  EXPECT_FALSE(scanner.start().valid()) << "Subsequent futures should be invalid";
}

}  // namespace psen_scan_v2_test

int main(int argc, char* argv[])
{
  testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
