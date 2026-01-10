#include "RaZ/Network/TcpClient.hpp"
#include "RaZ/Utils/Logger.hpp"

#include "asio/connect.hpp"
#include "asio/ip/tcp.hpp"
#include "asio/read.hpp"
#include "asio/streambuf.hpp"
#include "asio/write.hpp"

namespace Raz {

struct TcpClient::Impl {
  Impl() : socket(context), resolver(context) {}

  std::string read(const auto& readFunc, bool flush) {
    if (flush)
      buffer.consume(buffer.size());

    asio::error_code error;
    const std::size_t byteCount = readFunc(socket, buffer, error);

    if (error)
      throw std::runtime_error(std::format("[TcpClient] Failed to receive data: {}", error.message()));

    std::string result(static_cast<const char*>(buffer.data().data()), byteCount);
    buffer.consume(byteCount);

    return result;
  }

  asio::io_context context;
  asio::ip::tcp::socket socket;
  asio::ip::tcp::resolver resolver;
  asio::streambuf buffer;
};

TcpClient::TcpClient() : m_impl{ std::make_unique<Impl>() } {}

bool TcpClient::isConnected() const {
  return m_impl->socket.is_open();
}

void TcpClient::connect(const std::string& host, unsigned short port) {
  Logger::debug("[TcpClient] Connecting to {}:{}...", host, port);

  asio::error_code error;
  asio::connect(m_impl->socket, m_impl->resolver.resolve(host, std::to_string(port)), error);

  if (error)
    throw std::invalid_argument(std::format("[TcpClient] Failed to connect to {}:{}: {}", host, port, error.message()));

  Logger::debug("[TcpClient] Connected");
}

void TcpClient::send(const std::string& data) {
  Logger::debug("[TcpClient] Sending '{}'...", data);

  asio::error_code error;
  asio::write(m_impl->socket, asio::buffer(data), error);

  if (error)
    throw std::runtime_error(std::format("[TcpClient] Failed to send data: {}", error.message()));
}

std::size_t TcpClient::recoverAvailableByteCount() {
  asio::detail::io_control::bytes_readable command(true);
  m_impl->socket.io_control(command);
  return command.get();
}

std::string TcpClient::receiveAtLeast(std::size_t minByteCount, bool flush) {
  return m_impl->read([minByteCount] (asio::ip::tcp::socket& socket, asio::streambuf& buffer, asio::error_code& error) {
    // The buffer can already hold data received previously that will be returned; if necessary, reading only what is missing
    if (buffer.size() < minByteCount)
      asio::read(socket, buffer, asio::transfer_at_least(minByteCount - buffer.size()), error);

    return buffer.size();
  }, flush);
}

std::string TcpClient::receiveExactly(std::size_t byteCount, bool flush) {
  return m_impl->read([byteCount] (asio::ip::tcp::socket& socket, asio::streambuf& buffer, asio::error_code& error) {
    // The buffer can already hold data received previously that will be returned; if necessary, reading only what is missing
    if (buffer.size() < byteCount)
      asio::read(socket, buffer, asio::transfer_at_least(byteCount - buffer.size()), error);

    return byteCount;
  }, flush);
}

void TcpClient::disconnect() {
  if (!isConnected())
    return;

  Logger::debug("[TcpClient] Closing...");
  m_impl->socket.shutdown(asio::socket_base::shutdown_both);
  m_impl->socket.close();
  Logger::debug("[TcpClient] Closed");
}

TcpClient::~TcpClient() = default;

}
