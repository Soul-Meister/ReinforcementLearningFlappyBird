//
// Created by edward on 10/15/25.
//

#include "Network.h"

#include <iostream>
#include <random>

#include "../Layer/Layer.h"
#include <vector>
#include "../../main/config.h"
#include "../ActivationFunctions/LeakyReLU.h"

using namespace std;

Network::Network() {
    layers.clear();
    for (size_t i = 0; i < network_config.size(); ++i) {
        int fan_in = (i == 0) ? 7 : network_config[i - 1];
        layers.emplace_back(network_config[i], fan_in);
    }

}

void Network::init() {//HE init for leakyReLU
    std::random_device rd;
    std::mt19937 gen(rd());

    for (long unsigned int i = 0; i < layers.size(); i++) {//iterate through all layers
        int fan_in = 0;
        float variance = 0;
        if (i == 0) {
            fan_in = 7;//the amount of gamestate inputs; the "input" of the input neurons
        }
        else {
            fan_in = network_config[i-1];//get the fan in from previous layer
        }
        variance = (2)/(fan_in*(1+(alpha_config*alpha_config)));

        normal_distribution<float> dist(0.0, sqrt(variance));
        Layer &layer = layers.at(i);
        for (int j = 0; j < layers.at(i).get_layer_size(); j++) {//iterate through all neurons in the layer
            Neuron &neuron = layer.get_neurons().at(j);
            vector<float> &weights = neuron.get_weights();
            for (float &w : weights) {
                w = dist(gen);
                cout << w << " ";
            }
        }
    }
}


vector<float> Network::forward(const vector<float>& input) {
    last_input = input;
    vector<float> activations = input;
    LeakyReLU act;

    for (size_t k = 0; k < layers.size(); ++k) {
        Layer& layer = layers[k];
        std::vector<float> next(layer.get_layer_size());

        for (size_t i = 0; i < static_cast<size_t>(layer.get_layer_size()); ++i) {
            Neuron& neuron = layer.get_neurons()[i];
            const std::vector<float>& weights = neuron.get_weights();
            float z = neuron.get_bias();

            // Dot product: z = w·activations + b
            // (weights.size() should equal activations.size())
            for (size_t j = 0; j < weights.size(); ++j) {
                z += weights[j] * activations[j];
            }

            // Apply activation on hidden layers
            next[i] = (k < layers.size() - 1) ? act.activate(z) : z;

            neuron.z = z;
            neuron.a = (k < layers.size() - 1) ? act.activate(z) : z;
        }

        activations.swap(next); // move to next layer’s input
    }

    return activations; // final output vector (size = last layer size)
}

void Network::backward(const std::vector<float>& target) {
    LeakyReLU act;

    // ---------- Output layer ----------
    Layer& outputLayer = layers.back();
    for (size_t j = 0; j < outputLayer.get_layer_size(); ++j) {
        Neuron& neuron = outputLayer.get_neurons()[j];
        float error = neuron.a - target[j];
        neuron.delta = error * act.derivative(neuron.z);
    }

    // ---------- Hidden layers ----------
    for (int l = layers.size() - 2; l >= 0; --l) {
        Layer& current = layers[l];
        Layer& next = layers[l + 1];

        for (size_t j = 0; j < current.get_layer_size(); ++j) {
            float sum = 0.0f;
            for (size_t k = 0; k < next.get_layer_size(); ++k) {
                sum += next.get_neurons()[k].get_weights()[j] * next.get_neurons()[k].delta;
            }
            current.get_neurons()[j].delta = sum * act.derivative(current.get_neurons()[j].z);
        }
    }

    // ---------- Weight updates ----------
    for (size_t l = 0; l < layers.size(); ++l) {
        std::vector<float> prev_activations;
        if (l == 0)
            prev_activations = last_input;  // store this from forward()
        else
            prev_activations = layers[l - 1].get_activations();

        for (Neuron& neuron : layers[l].get_neurons()) {
            for (size_t w = 0; w < neuron.get_weights().size(); ++w) {
                neuron.get_weights()[w] -= learning_rate_config * neuron.delta * prev_activations[w];
            }
            neuron.set_bias(neuron.get_bias() - learning_rate_config * neuron.delta);
        }
    }
}






