//
// Created by edward on 10/15/25.
//

#ifndef FLAPPYBIRD_NETWORK_H
#define FLAPPYBIRD_NETWORK_H

#include <string>
#include <vector>
#include "../Layer/Layer.h"

class Network {
private:
    vector<Layer> layers;
    vector<float> last_input;
public:
    Network();
    void init();
    vector<float> forward(const vector<float> &input);
    void backward(const vector<float>& target);
    int get_layer_num();
    void add_layer(vector<Neuron> layer);

    void export_network(string filename);
};


#endif //FLAPPYBIRD_NETWORK_H