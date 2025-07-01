# auhub

一个 C++ 音频播放和处理库，专注于提供灵活的音频源和播放目标选项。

## 特性 (Features)

*   **灵活的音频源**: 支持从本地文件加载音频 (通过 `libsndfile`)，也支持接收实时音频流 (例如通过 WebSocket)。
*   **多样的播放目标**: 支持通过标准声卡输出 (`PortAudio`)，以及通过 `UART` 接口输出音频。
*   **可扩展设计**: `AudioBase` 和 `PlayerBase` 基类方便用户自定义新的音频源和播放器实现。
*   **现代 C++**: 使用 C++20 标准编写，确保代码的现代性和效率。
*   **实用工具集成**:
    *   使用 `spdlog` 进行结构化日志记录。
    *   示例代码中使用 `cxxopts` 进行命令行参数解析。

## 依赖 (Dependencies)

本项目依赖以下第三方库：

*   **`libsndfile`**: 用于读取和写入多种格式的音频文件。
*   **`PortAudio`**: 一个跨平台的音频 I/O 库，用于通过声卡播放和录制音频。
*   **`spdlog`**: 一个快速、仅头文件的 C++ 日志库。
*   **`cxxopts`**: (用于示例) 一个轻量级的 C++ 命令行参数解析库。

我们推荐使用 [vcpkg](https://github.com/microsoft/vcpkg) 来管理这些 C++ 依赖。

### 系统依赖

在开始构建前，请确保已安装以下系统级依赖 (以 Debian/Ubuntu 为例)：

```sh
sudo apt install -y libsndfile1-dev portaudio19-dev
```

### 通过 vcpkg 安装 C++ 依赖

```sh
vcpkg install spdlog cxxopts
```
> **注意**：`libsndfile` 和 `PortAudio` 通常作为系统库安装。如果您的系统中已通过包管理器 (如 `apt`) 安装了这些库，CMake 应该能够找到它们。如果 vcpkg 中也安装了这些库的版本，请确保 CMake 正确链接到您期望的版本。通常，优先使用系统提供的版本。

## 构建指南 (Building)

1.  **确保依赖已安装**:
    请参照上一节“依赖 (Dependencies)”确保所有系统和 C++ 库依赖均已正确安装。

2.  **配置 CMake**:
    项目使用 CMake 进行构建。推荐使用 `CMakePresets.json` 中定义的预设配置。
    ```sh
    cmake --preset ninja-multi-vcpkg
    ```
    此命令会使用 Ninja 作为构建系统，并配置 vcpkg 工具链。

3.  **编译项目**:
    ```sh
    cmake --build --preset ninja-vcpkg-release
    ```
    或者，编译 Debug 版本：
    ```sh
    cmake --build --preset ninja-vcpkg-debug
    ```

构建完成后，库文件 (`libauhub.a` 或 `auhub.lib`) 将位于 `build/lib/` (或类似路径，取决于您的构建配置，例如 `build/ninja-vcpkg-release/lib/`) 目录下，示例可执行文件将位于相应的 `examples` 子目录下 (例如 `build/ninja-vcpkg-release/examples/`)。

## 使用方法 (Usage)

### 1. 作为库在您的项目中使用

要在您自己的 CMake 项目中使用 `auhub`，您需要：

1.  确保 `auhub` 已经编译，并且您的项目可以找到其头文件 (`include` 目录) 和库文件。
    一种常见做法是将 `auhub` 作为 CMake 子目录 (e.g., using `add_subdirectory`) 集成到您的项目中，或者使用 `find_package` (如果 `auhub` 安装到了系统路径或提供了相应的 CMake 配置文件)。

2.  在您的 `CMakeLists.txt` 中链接到 `auhub` 库：

    ```cmake
    # 如果使用 add_subdirectory
    add_subdirectory(path/to/auhub)

    # 链接到 auhub 库 (auhub 是在 CMakeLists.txt 中定义的目标名)
    target_link_libraries(your_target_name PRIVATE auhub)
    ```

下面是一个简单的 C++ 代码示例，演示如何使用 `auhub` 从文件加载音频并使用声卡播放：

```cpp
#include "auhub/audio/file_audio.h"
#include "auhub/player/card_player.h"
#include <spdlog/spdlog.h>
#include <thread> // For std::this_thread::sleep_for
#include <chrono> // For std::chrono::seconds

int main(int argc, char* argv[]) {
    if (argc < 2) {
        spdlog::error("用法: {} <音频文件路径>", argv[0]);
        return 1;
    }
    std::string audio_file_path = argv[1];

    spdlog::set_level(spdlog::level::debug);

    // 创建 FileAudio 对象来加载音频文件
    std::unique_ptr<auhub::audio::AudioBase> audio =
        auhub::audio::AudioBase::create<auhub::audio::FileAudio>(audio_file_path);

    if (!audio) {
        spdlog::error("无法加载音频文件: {}", audio_file_path);
        return 1;
    }

    // 创建 CardPlayer 对象
    std::unique_ptr<auhub::player::PlayerBase> player =
        std::make_unique<auhub::player::CardPlayer>();

    // 开始播放
    player->play(std::move(audio)); // audio 现在为空

    // 播放直到音频结束或发生错误
    // 实际应用中，你可能需要更复杂的逻辑来处理播放状态
    while(player->isPlaying()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // player->stop(); // stop 会在播放完成或析构时自动调用，但显式调用也无妨
    spdlog::info("播放流程结束。");

    return 0;
}
```
> **编译和运行示例代码**:
> 将上述代码保存为 `main.cpp`，并创建一个简单的 `CMakeLists.txt`：
> ```cmake
> cmake_minimum_required(VERSION 3.16)
> project(AuHubUserExample)
> set(CMAKE_CXX_STANDARD 20)
>
> # 假设 auhub 项目与此示例项目在同一父目录下
> add_subdirectory(../auhub ../auhub_build) # 第二个参数是 auhub 的构建目录
>
> add_executable(my_player main.cpp)
> target_link_libraries(my_player PRIVATE auhub spdlog::spdlog)
> ```
> 然后编译并运行: `cmake -S . -B build && cmake --build build && ./build/my_player path/to/your/audiofile.wav`


### 2. 运行项目内建示例

项目包含一些示例来演示 `auhub` 的功能。编译后，它们通常位于构建目录下的 `examples` 子目录中。

*   **本地文件播放 (`test_local_file`)**:
    此示例演示了如何从本地音频文件加载并播放。
    运行方式 (假设您在构建目录下)：
    ```sh
    ./examples/test_local_file -f /path/to/your/audio.wav -t card
    ```
    *   `-f, --filename`: 指定要播放的音频文件路径。
    *   `-t, --player_type`: 指定播放器类型。可以是 `card` (声卡播放) 或 `uart` (UART输出)。

*   **WebSocket 音频流**:
    *   `examples/ws_stream_player`: 一个 C++ 实现的 WebSocket 服务端，用于接收音频流并播放。
    *   `examples/py_stream_client/audio_ws_client.py`: 一个 Python 脚本，作为 WebSocket 客户端，将本地 WAV 文件流式传输到服务端。
    这两个示例共同演示了通过网络流式传输和播放音频的能力。请查阅其各自目录下的代码以获取更多运行细节。

## 目录结构

```text
auhub/
├── .github/                # GitHub Actions 配置文件
├── 3rdparty/               # 第三方依赖库 (例如 uart)
├── CMakeLists.txt          # 主 CMake 构建脚本
├── CMakePresets.json       # CMake 预设配置
├── examples/               # 示例代码
│   ├── py_stream_client/   # Python WebSocket 音频流客户端示例
│   ├── test_local_file.cc  # 本地文件播放示例
│   └── ws_stream_player/   # C++ WebSocket 音频流服务端示例
├── include/                # 公共头文件
│   └── auhub/
│       ├── audio/          # 音频源相关接口和实现
│       └── player/         # 播放器相关接口和实现
├── readme.md               # 本文档
├── src/                    # 源代码实现
│   ├── audio/              # 音频源实现 (.cc 文件)
│   └── player/             # 播放器实现 (.cc 文件)
├── test/                   # 测试代码 (当前内容较少)
├── vcpkg                   # vcpkg Git 子模块
└── vcpkg.json              # vcpkg 配置文件 (定义项目级依赖)
```
