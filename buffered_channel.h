#ifndef BUFFERED_CHANNEL_BUFFERED_CHANNEL_H
#define BUFFERED_CHANNEL_BUFFERED_CHANNEL_H

#include <queue>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <thread>

template<class T>
class buffered_channel {
public:
    explicit buffered_channel(int size) : buffer_size(size), isClose(false) {}

    void Send(T value) {
        if (isClose) {
            throw new std::runtime_error("Channel has been closed");
        }

        std::unique_lock<std::mutex> lock(mutex);
        condition_variable.wait(lock, [&] { return channel.size() < buffer_size; });
        T value1 = value;
        channel.push(std::move(value));
        condition_variable.notify_one();

    }

    std::pair<T, bool> Recv() {
        if (isClose) {
            return std::make_pair(T(), false);
        }

        std::unique_lock<std::mutex> lock(mutex);
        condition_variable.wait(lock, [&] { return !channel.empty(); });
        T recv = channel.front();
        channel.pop();
        condition_variable.notify_one();
        return std::make_pair(recv, true);
    }

    void Close() {
        isClose = true;
        condition_variable.notify_all();
    }

private:
    int buffer_size;
    bool isClose;
    std::queue<T> channel;
    std::mutex mutex;
    std::condition_variable condition_variable;
};


#endif