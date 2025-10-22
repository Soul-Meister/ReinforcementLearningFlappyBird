//
// Created by edward on 10/16/25.
//

#ifndef FLAPPYBIRD_LAYER_H
#define FLAPPYBIRD_LAYER_H

#include <vector>
#include "../Nueron/Neuron.h"

using namespace std;

class Layer {
private:
    vector<Neuron> neurons;
public:
    int get_layer_size();
};


#endif //FLAPPYBIRD_LAYER_H