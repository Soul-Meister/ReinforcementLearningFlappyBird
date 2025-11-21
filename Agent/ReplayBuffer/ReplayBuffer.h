#ifndef FLAPPYBIRD_REPLAYBUFFER_H
#define FLAPPYBIRD_REPLAYBUFFER_H

#include <condition_variable>
#include <mutex>
#include <random>
#include <vector>

class ReplayBuffer {
public:
    struct Transition {
        std::vector<float> state;
        int action;
        double reward;
        std::vector<float> next_state;
        bool done;
    };

    explicit ReplayBuffer(size_t capacity);

    void push(Transition&& t);
    void wait_until_size(size_t n);
    size_t size() const;
    std::vector<Transition> sample(size_t batch, std::mt19937& rng);

private:
    size_t cap;
    std::vector<Transition> data;
    size_t size_;
    size_t head;
    mutable std::mutex mu;
    std::condition_variable cv;
};

#endif // FLAPPYBIRD_REPLAYBUFFER_H
