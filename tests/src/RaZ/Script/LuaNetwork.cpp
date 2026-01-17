#include "TestUtils.hpp"

#include <catch2/catch_test_macros.hpp>

TEST_CASE("LuaNetwork TcpClient", "[script][lua][network]") {
  CHECK(TestUtils::executeLuaScript(R"(
    local tcpClient = TcpClient.new()
    --tcpClient       = TcpClient.new("localhost", 1234) -- Throws as there's no available server

    --tcpClient:connect("localhost", 1234) -- Throws as there's no available server
    assert(not tcpClient:isConnected())
    -- Data transfer functions throw when not connected
    --tcpClient:send("data")
    --tcpClient:recoverAvailableByteCount()
    --tcpClient:receive()
    --tcpClient:receiveAtLeast(1)
    --tcpClient:receiveExactly(1)
    --tcpClient:receiveUntil("\0")
    tcpClient:disconnect()
  )"));
}

TEST_CASE("LuaNetwork TcpServer", "[script][lua][network]") {
  CHECK(TestUtils::executeLuaScript(R"(
    local tcpServer = TcpServer.new()

    tcpServer:start(1234)
    tcpServer:stop()
  )"));
}
