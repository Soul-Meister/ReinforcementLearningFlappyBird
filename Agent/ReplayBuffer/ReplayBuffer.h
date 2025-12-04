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
        float priority;
    };

    explicit ReplayBuffer(size_t capacity);

    void push(Transition&& t);
    void wait_until_size(size_t n);
    size_t size() const;
    std::vector<Transition> sample(size_t batch, std::mt19937& rng);
    std::vector<Transition> sample_prioritized(size_t batch, std::mt19937& rng);
    void update_priority(size_t idx, float new_p);


private:
    size_t cap;
    std::vector<Transition> data;
    size_t size_;
    size_t head;
    mutable std::mutex mu;
    std::condition_variable cv;


    float alpha = 0.6f;    // PER exponent
    float eps_p = 1e-4f;   // small constant to avoid zero priority
    float max_priority = 1.0f; // track max to give new samples high chance
};

#endif // FLAPPYBIRD_REPLAYBUFFER_H
