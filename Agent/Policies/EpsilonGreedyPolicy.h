//
// Created by edward on 10/15/25.
//

#ifndef FLAPPYBIRD_EPSILONGREEDYPOLICY_H
#define FLAPPYBIRD_EPSILONGREEDYPOLICY_H

#include "../../main/config.h"


class EpsilonGreedyPolicy {
public:
    EpsilonGreedyPolicy(float, float);
    float getEpsilon();
    void decay();
private:
    float epsilon;
    float decay_val;
    float min_val;

};


#endif //FLAPPYBIRD_EPSILONGREEDYPOLICY_H