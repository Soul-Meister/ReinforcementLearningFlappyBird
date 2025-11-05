//
// Created by edward on 10/15/25.
//

#include "EpsilonGreedyPolicy.h"


EpsilonGreedyPolicy::EpsilonGreedyPolicy(float decay_, float min) {
    epsilon = 1;
    decay_val = decay_;
    min_val = min;
}

void EpsilonGreedyPolicy::decay() {
    epsilon -= decay_val;

    if (epsilon < min_val) {
        epsilon = min_val;
    }
}

float EpsilonGreedyPolicy::getEpsilon() {
    return epsilon;
}
