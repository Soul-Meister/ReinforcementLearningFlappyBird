//
// Created by edward on 10/15/25.
//

#include "Neuron.h"
#include "../../main/config.h"


#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <vector>

using namespace std;

Neuron::Neuron(int weights_size) {
    weights = vector<float>(weights_size);
    learning_rate = learning_rate_config;
    bias = bias_config;
}

void Neuron::set_weights() {
    weights.clear();
}

float Neuron::get_bias() const {
    return bias;
}

void Neuron::set_bias(float val) {
    bias = val;
}

vector<float>& Neuron::get_weights() {
    return weights;
}
