#include "RaZ/Network/UdpServer.hpp"

#include <catch2/catch_test_macros.hpp>

#include <thread>

TEST_CASE("UdpServer basic", "[network]") {
  Raz::UdpServer server;

  CHECK_NOTHROW(server.stop()); // Stopping a non-running server isn't an error

  CHECK_NOTHROW(server.start(1234));
  CHECK_NOTHROW(server.start(1234)); // Starting an already running server restarts it properly

  CHECK_NOTHROW(server.stop());
  CHECK_NOTHROW(server.stop()); // Stopping an already stopped server does nothing
}
