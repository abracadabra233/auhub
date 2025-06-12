### deps

```sh
sudo apt install -y tmux libsndfile1-dev portaudio19-dev

vcpkg install spdlog
```

```sh
cmake --preset ninja-multi-vcpkg
cmake --build --preset ninja-vcpkg-release
```
