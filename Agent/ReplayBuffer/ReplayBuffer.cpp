#include "ReplayBuffer.h"

#include <vector>
#include <mutex>
#include <random>
#include <atomic>

using namespace std;

ReplayBuffer::ReplayBuffer(size_t capacity)
    : cap(capacity),
      data(capacity),     // FIX: must allocate vector(size)
      size_(0),
      head(0)
{
}

void ReplayBuffer::push(Transition&& t) {
    std::lock_guard<std::mutex> lock(mu);

    data[head] = std::move(t);      // FIX: remove std::data and use data[]
    head = (head + 1) % cap;

    if (size_ < cap)
        ++size_;

    cv.notify_one();
}

void ReplayBuffer::wait_until_size(size_t n) {
    std::unique_lock<std::mutex> lock(mu);
    cv.wait(lock, [&]{ return size_ >= n; });
}

std::vector<ReplayBuffer::Transition>
ReplayBuffer::sample(size_t batch, std::mt19937& rng) {
    std::lock_guard<std::mutex> lock(mu);

    std::uniform_int_distribution<size_t> dist(0, size_ - 1);

    std::vector<Transition> out;
    out.reserve(batch);

    for (size_t i = 0; i < batch; ++i)
        out.push_back(data[dist(rng)]);

    return out;
}

size_t ReplayBuffer::size() const {
    std::lock_guard<std::mutex> lock(mu);
    return size_;
}
