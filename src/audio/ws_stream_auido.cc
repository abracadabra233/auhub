#include "auhub/audio/ws_stream_auido.h"

namespace auhub {
namespace audio {

auto WsStreamAudio::parse_msg_to_audio(beast::flat_buffer &buffer) {
  const auto *data = reinterpret_cast<const short *>(buffer.data().data());
  return std::vector<short>(data, data + buffer.data().size() / 2);
}

void WsStreamAudio::do_ws_accept(
    beast::error_code ec,
    std::shared_ptr<websocket::stream<beast::tcp_stream>> ws) {
  if (ec) {
    do_accept();
    return spdlog::error("WebSocket accept error: {}", ec.message());
  }

  // ready_.release();
  spdlog::info("stream audio ready!");

  auto buffer = std::make_shared<beast::flat_buffer>();
  do_ws_read(ws, buffer);
}

void WsStreamAudio::do_ws_closed() {
  //? 不要push true ,那么这个就一直不会结束
  // todo ，只在ws连接上之后player 才开始消费
  // push({}, true);
  spdlog::info("single stream audio closed!");
  do_accept();
}

void WsStreamAudio::do_ws_read(
    std::shared_ptr<websocket::stream<beast::tcp_stream>> ws,
    std::shared_ptr<beast::flat_buffer> buffer) {
  ws->async_read(*buffer,
                 [this, ws, buffer](beast::error_code ec, size_t bytes) {
                   this->on_ws_read(ws, buffer, ec, bytes);
                 });
}

void WsStreamAudio::on_ws_read(
    std::shared_ptr<websocket::stream<beast::tcp_stream>> ws,
    std::shared_ptr<beast::flat_buffer> buffer, beast::error_code ec, size_t) {
  if (ec == websocket::error::closed) {
    spdlog::info("ws client closed normally");
    return do_ws_closed();
  }
  if (ec) {
    spdlog::warn("ws client closed with error: {}", ec.message());
    return do_ws_closed();
  }

  push(parse_msg_to_audio(*buffer), false);
  buffer->consume(buffer->size());
  do_ws_read(ws, buffer);
}

void WsStreamAudio::do_accept() {
  acceptor_.async_accept(
      asio::make_strand(ioc_),
      beast::bind_front_handler(&WsStreamAudio::on_accept, this));
}

void WsStreamAudio::on_accept(beast::error_code ec, tcp::socket socket) {
  if (ec) {
    do_accept();
    return spdlog::error("on socket accept error: {}", ec.message());
  }

  auto ws =
      std::make_shared<websocket::stream<beast::tcp_stream>>(std::move(socket));
  ws->set_option(
      websocket::stream_base::timeout::suggested(beast::role_type::server));
  ws->async_accept(
      [this, ws](beast::error_code ec) { this->do_ws_accept(ec, ws); });

  //? 同一时刻只处理一个连接
  // do_accept();
}

WsStreamAudio::WsStreamAudio(int ws_port, int sampleRate, int channels,
                             size_t buffer_size)
    : StreamAudio(sampleRate, channels, buffer_size),
      ioc_(1),
      acceptor_(ioc_),
      work_guard_(asio::make_work_guard(ioc_)) {
  run(ws_port);

  work_thread_ = std::thread([this]() { ioc_.run(); });
}

WsStreamAudio::~WsStreamAudio() {
  ioc_.stop();
  work_guard_.reset();
  work_thread_.join();
  spdlog::info("WsStreamAudio stop && cleanup");
}

void WsStreamAudio::run(uint16_t port) {
  try {
    auto const address = asio::ip::make_address("0.0.0.0");
    beast::error_code ec;

    acceptor_.open(tcp::v4(), ec);
    if (ec) {
      spdlog::error("Acceptor open error: {}", ec.message());
      return;
    }

    acceptor_.set_option(asio::socket_base::reuse_address(true), ec);
    if (ec) {
      spdlog::error("Set option error: {}", ec.message());
      return;
    }

    acceptor_.bind(tcp::endpoint(address, port), ec);
    if (ec) {
      spdlog::error("Bind error: {}", ec.message());
      return;
    }

    acceptor_.listen(asio::socket_base::max_listen_connections, ec);
    if (ec) {
      spdlog::error("Listen error: {}", ec.message());
      return;
    }

    do_accept();

  } catch (const std::exception &e) {
    spdlog::error("Error: {}", e.what());
  }
}
};  // namespace audio
}  // namespace auhub