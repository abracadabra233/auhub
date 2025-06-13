### deps

```sh
sudo apt install -y tmux libsndfile1-dev portaudio19-dev

vcpkg install spdlog
vcpkg install cxxopts
vcpkg install "boost-asio"
vcpkg install "boost-beast"
vcpkg install "boost-container"
```

```sh
cmake --preset ninja-multi-vcpkg
cmake --build --preset ninja-vcpkg-release
```
