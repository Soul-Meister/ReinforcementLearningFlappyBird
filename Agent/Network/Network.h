//
// Created by edward on 10/15/25.
//

#ifndef FLAPPYBIRD_NETWORK_H
#define FLAPPYBIRD_NETWORK_H

#include <string>
#include <vector>
#include "../Nueron/Neuron.h"
#include "../Layer/Layer.h"

class Network {
private:
    vector<vector<Neuron>> layers;
public:
    Network();
    int get_layer_num();
    void add_layer(vector<Neuron> layer);

    void export_network(string filename);
};


#endif //FLAPPYBIRD_NETWORK_H