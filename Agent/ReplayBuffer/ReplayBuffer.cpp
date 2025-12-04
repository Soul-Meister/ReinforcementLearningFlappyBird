#include "ReplayBuffer.h"

#include <vector>
#include <mutex>
#include <random>
#include <atomic>

using namespace std;

struct Transition {
    std::vector<float> state;
    int action;
    float reward;
    std::vector<float> next_state;
    bool done;

    float priority = 1.0f;
};



ReplayBuffer::ReplayBuffer(size_t capacity) {
      cap = capacity;
      data = vector<Transition>(capacity);
      size_ = 0;
      head = 0;
}

void ReplayBuffer::push(Transition&& t) {
    std::lock_guard<std::mutex> lock(mu);

    // Ensure a reasonable priority on insert.
    // If caller didn't set t.priority, use current max_priority (or 1.0f).
    if (t.priority <= 0.0f) {
        t.priority = (max_priority > 0.0f) ? max_priority : 1.0f;
    }

    data[head] = std::move(t);

    // track max priority so new transitions stay competitive
    if (data[head].priority > max_priority) {
        max_priority = data[head].priority;
    }

    head = (head + 1) % cap;

    if (size_ < cap)
        ++size_;

    cv.notify_one();
}


void ReplayBuffer::wait_until_size(size_t n) {
    std::unique_lock<std::mutex> lock(mu);
    cv.wait(lock, [&]{ return size_ >= n; });
}

std::vector<ReplayBuffer::Transition> ReplayBuffer::sample(size_t batch, std::mt19937& rng) {
    std::lock_guard<std::mutex> lock(mu);

    if (size_ == 0 || size_ < batch)
        return {};

    std::uniform_int_distribution<size_t> dist(0, size_ - 1);

    std::vector<Transition> out;
    out.reserve(batch);

    for (size_t i = 0; i < batch; ++i)
        out.push_back(data[dist(rng)]);

    return out;
}

std::vector<ReplayBuffer::Transition> ReplayBuffer::sample_prioritized(size_t batch, std::mt19937& rng)
{
    std::lock_guard<std::mutex> lock(mu);

    if (size_ == 0 || size_ < batch)
        return {};

    // 1) Build priorities^alpha and their sum
    std::vector<float> p(size_);
    float sum_p = 0.0f;

    for (size_t i = 0; i < size_; ++i) {
        float base = data[i].priority + eps_p;
        float pa   = std::pow(base, alpha);
        p[i] = pa;
        sum_p += pa;
    }

    std::uniform_real_distribution<float> dist(0.0f, sum_p);

    std::vector<Transition> out;
    out.reserve(batch);

    // 2) Draw 'batch' samples by walking the cumulative distribution
    for (size_t b = 0; b < batch; ++b) {
        float r = dist(rng);
        float cumsum = 0.0f;

        for (size_t i = 0; i < size_; ++i) {
            cumsum += p[i];
            if (cumsum >= r) {
                out.push_back(data[i]);
                break;
            }
        }
    }

    return out;
}

void ReplayBuffer::update_priority(size_t idx, float new_p) {
    std::lock_guard<std::mutex> lock(mu);
    if (idx >= size_) return;

    data[idx].priority = std::max(new_p, eps_p);
    if (data[idx].priority > max_priority) {
        max_priority = data[idx].priority;
    }
}





size_t ReplayBuffer::size() const {
    std::lock_guard<std::mutex> lock(mu);
    return size_;
}
