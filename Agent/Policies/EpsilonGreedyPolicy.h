//
// Created by edward on 10/15/25.
//

#ifndef FLAPPYBIRD_EPSILONGREEDYPOLICY_H
#define FLAPPYBIRD_EPSILONGREEDYPOLICY_H

#include "../../main/Config.h"


class EpsilonGreedyPolicy {
public:
    EpsilonGreedyPolicy(float, float);
    float getEpsilon();
    void decay();
    float epsilon;
private:
    float decay_val;
    float min_val;

};


#endif //FLAPPYBIRD_EPSILONGREEDYPOLICY_H