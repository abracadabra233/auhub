### deps

```sh
sudo apt install -y tmux libsndfile1-dev portaudio19-dev

vcpkg install spdlog
vcpkg install cxxopts
# vcpkg install "boost-asio"
# vcpkg install "boost-beast"
# vcpkg install "boost-container"
```

```sh
cmake --preset ninja-multi-vcpkg
cmake --build --preset ninja-vcpkg-release
```

# todo

- 目前 ws stream 只能启动一个,stop 之后在启动会错

  - 正常逻辑应该是 ws 服务启动之后一直都在
  - stop 只是 stop 掉 ws 服务

- 播放的线程在结束后需要 join,一次播放就一次 join，而不是应该这次播放取 join 上一次的

- 该项目目前值只能支持，16kHz,16bit,单声道的音频源以及流式音频源，file_audio 还只能是 wav 格式，请帮我完善一下，支持其他格式和音频配置，比如 mp3,flac ，并且扩展这个库的接口，你可以引入其他库，最好是通过 vcpkg 引入。你觉得之前的代码不优雅的地方也可以大刀阔斧的修改，你随意发挥
