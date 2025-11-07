//
// Created by edward on 10/16/25.
//

#include "Layer.h"
Layer::Layer(int size, int weights_size) {

    neurons.clear();
    for (int i = 0; i < size; i++) {
            neurons.push_back(Neuron(weights_size));
    }
}
Layer::Layer(const Layer& other) {
    if (this != &other) {
        neurons = other.neurons;
    }
}

Layer& Layer::operator=(const Layer& other) {
    if (this != &other) {
        neurons = other.neurons;
    }
    return *this;
}


int Layer::get_layer_size() {
    return neurons.size();
}

vector<Neuron>& Layer::get_neurons() {
    return neurons;
}

vector<float> Layer::get_activations() const {
    std::vector<float> activations;
    activations.reserve(neurons.size());
    for (const Neuron& n : neurons)
        activations.push_back(n.a);
    return activations;
}
