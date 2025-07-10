#pragma once

#include <thread>

#include "auhub/audio/base.h"
#include "auhub/player/play_progress.h"

namespace auhub {
namespace player {

/**
 * @brief 播放器基类（使用CRTP模式）
 *
 * 提供音频播放的基本框架，包括线程管理、播放控制等功能。
 * 派生类需要实现纯虚函数play_()来定义具体播放逻辑。
 *
 * @tparam T 派生类类型（CRTP模式）
 */
template <typename T>
class PlayerBase {
 private:
  // 禁用拷贝构造和赋值操作，确保单例特性
  PlayerBase(const PlayerBase &) = delete;
  PlayerBase &operator=(const PlayerBase &) = delete;

  // 播放线程，负责在后台执行音频播放
  std::thread play_thread_;

 protected:
  // 允许子类通过默认构造函数实例化
  PlayerBase() = default;

  // 析构函数：确保线程在对象销毁前正确结束
  ~PlayerBase() {
    if (play_thread_.joinable()) {
      play_thread_.join();
    }
  };

  // 静态原子标志，指示当前是否正在播放
  static std::atomic<bool> playing_;

  // 纯虚函数：由子类实现具体的播放逻辑
  // @param audio 音频数据指针
  // @param progress 播放进度跟踪器
  // @return 播放成功返回true，否则返回false
  virtual bool play_(audio::AudioBase *audio,
                     std::shared_ptr<PlayProgress> progress) = 0;

 public:
  /**
   * @brief 获取播放器单例实例
   *
   * 使用 Meyers 单例模式，线程安全地创建唯一实例。
   *
   * @tparam Args 构造函数参数类型
   * @param args 转发给子类构造函数的参数
   * @return std::shared_ptr<PlayerBase> 单例实例的共享指针
   */
  template <typename... Args>
  static std::shared_ptr<PlayerBase> getInstance(Args &&...args) {
    static auto instance = std::make_shared<T>(std::forward<Args>(args)...);
    return instance;
  }

  /**
   * @brief 异步播放音频
   *
   * 在新线程中执行音频播放，支持unique_ptr或shared_ptr类型的音频源。
   * 如果已有音频正在播放，会先停止当前播放。
   *
   * @tparam AudioPtr 音频指针类型（必须是unique_ptr或shared_ptr）
   * @param audio 音频数据智能指针
   * @param progress 可选的播放进度跟踪器
   */
  template <typename AudioPtr>
    requires(std::is_same_v<AudioPtr, std::unique_ptr<audio::AudioBase>> ||
             std::is_same_v<AudioPtr, std::shared_ptr<audio::AudioBase>>)
  void play(AudioPtr audio, std::shared_ptr<PlayProgress> progress = nullptr) {
    if (!audio) {
      spdlog::error("play audio failed: audio is null");
      return;
    }
    if (playing_.load()) {
      spdlog::warn("audio is playing,stop first!");
      stop();
    }
    if (play_thread_.joinable()) {
      play_thread_.join();
    }

    auto play_warp = [this, progress, audio = std::move(audio)]() mutable {
      playing_.store(true);
      play_(audio.get(), progress);
      playing_.store(false);
    };

    play_thread_ = std::thread(std::move(play_warp));
  }

  /**
   * @brief 停止当前播放
   *
   * 设置播放标志为false并等待播放线程结束。
   */
  void stop() {
    if (playing_.load()) {
      playing_.store(false);
    }
    if (play_thread_.joinable()) {
      play_thread_.join();
    }
  }
};

// 初始化静态成员变量
template <typename T>
std::atomic<bool> PlayerBase<T>::playing_ = false;

}  // namespace player
}  // namespace auhub