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
    vector<float> get_activations() const;
    Layer(int size, int weights_size);

    Layer(const Layer &other);

    Layer();

    void add_neuron(Neuron neuron);

    Layer &operator=(const Layer &other);

    vector<Neuron>& get_neurons();
    int get_layer_size();
};


#endif //FLAPPYBIRD_LAYER_H