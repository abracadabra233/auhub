#pragma once

#include <mutex>
#include <condition_variable>
#include <memory>
#include <chrono>
#include <optional>

template <typename T>
class BlockingValue
{
private:
    mutable std::mutex mutex_;   // 保护共享数据的互斥锁
    std::condition_variable cv_; // 用于线程间通知的条件变量
    std::unique_ptr<T> value_;   // 存储的值，使用 std::unique_ptr 表示值是否存在
    bool is_ready_ = false;      // 标记值是否已设置

public:
    BlockingValue() = default;

    // 禁止拷贝构造和拷贝赋值
    BlockingValue(const BlockingValue &) = delete;
    BlockingValue &operator=(const BlockingValue &) = delete;

    // 允许移动构造和移动赋值
    BlockingValue(BlockingValue &&) = default;
    BlockingValue &operator=(BlockingValue &&) = default;

    // 设置值并通知等待的线程
    void Set(std::unique_ptr<T> value)
    {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            value_ = std::move(value); // 使用移动语义避免不必要的拷贝
            is_ready_ = true;
        }
        cv_.notify_all(); // 通知所有等待的线程
    }

    // 等待并获取值（阻塞直到值被设置）
    std::unique_ptr<T> Get()
    {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [this]
                 { return is_ready_; }); // 等待条件满足
        is_ready_ = false;
        return std::move(value_);        // 返回值的移动副本
    }

    // 带超时的等待并获取值
    std::unique_ptr<T> Get(const int timeout)
    {
        std::unique_lock<std::mutex> lock(mutex_);
        if (cv_.wait_for(lock, std::chrono::milliseconds(timeout), [this]
                         { return is_ready_; }))
        {
            is_ready_ = false;
            return std::move(value_); // 返回值（如果超时则返回空）
        }
        return nullptr; // 超时返回空
    }

    // 检查值是否已设置
    bool IsReady() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return is_ready_;
    }
};
