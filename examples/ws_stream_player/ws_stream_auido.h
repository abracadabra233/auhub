#pragma once

#include <boost/asio.hpp>
#include <boost/beast.hpp>

#include "auhub/audio/stream_auido.h"

namespace beast = boost::beast;
namespace asio = boost::asio;
namespace websocket = beast::websocket;
using tcp = asio::ip::tcp;

namespace auhub {
namespace audio {

class WsStreamAudio : public StreamAudio {
 protected:
 public:
  WsStreamAudio(int ws_port, int sampleRate, int channels);

  ~WsStreamAudio();

  void run(uint16_t port);

 private:
  asio::io_context ioc_;
  asio::executor_work_guard<asio::io_context::executor_type> work_guard_;
  std::thread work_thread_;
  tcp::acceptor acceptor_;

  void do_ws_accept(beast::error_code ec,
                    std::shared_ptr<websocket::stream<beast::tcp_stream>> ws);

  void do_ws_closed();

  void do_ws_read(std::shared_ptr<websocket::stream<beast::tcp_stream>> ws,
                  std::shared_ptr<beast::flat_buffer> buffer);

  void on_ws_read(std::shared_ptr<websocket::stream<beast::tcp_stream>> ws,
                  std::shared_ptr<beast::flat_buffer> buffer,
                  beast::error_code ec, size_t);

  void do_accept();

  void on_accept(beast::error_code ec, tcp::socket socket);

  static auto parse_msg_to_audio(beast::flat_buffer& buffer);
};
}  // namespace audio
}  // namespace auhub