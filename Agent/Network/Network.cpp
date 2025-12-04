//
// Created by edward on 10/15/25.
//

#include "Network.h"

#include <filesystem>
#include <iostream>
#include <random>
#include <vector>
#include <fstream>

#include "../Layer/Layer.h"
#include "../../main/Config.h"
#include "../ActivationFunctions/LeakyReLU.h"

using namespace std;

Network::Network(int mode) {
    if (mode == 0) {
        layers.clear();
        for (size_t i = 0; i < network_config.size(); ++i) {
            int fan_in = (i == 0) ? 7 : network_config[i - 1];
            layers.emplace_back(network_config[i], fan_in);
        }
    }
    if (mode == 1) {
        layers.clear();
    }
}

Network::Network(const Network& other) {
    if (this != &other) {
        scoped_lock lock(mtx, other.mtx);
        layers = other.layers;
        last_input = other.last_input; // include if you have this in the class
    }
}

void Network::save_model(Network* network) {
    lock_guard<mutex> lock(mtx);

    // Ensure the directory exists
    filesystem::create_directory("models");

    for (int i = 0; i < 30; i++) {
        string path = "models/Model_" + to_string(i) + ".txt";

        // 1. Check if file already exists
        if (filesystem::exists(path)) {
            continue;  // try next index
        }

        // 2. Try to create the file
        ofstream fout(path);
        if (!fout.is_open()) {
            cout << "ERROR: Could not create file '" << path
                 << "'. Check folder permissions.\n";
            return;
        }

        cout << "Created model file: " << path << "\n";

        // 3. Dump network structure
        for (Layer &layer : network->layers) {
            fout << "LAYER\n";
            for (Neuron &neuron : layer.get_neurons()) {
                fout << " NEURON\n";
                for (float weight : neuron.get_weights()) {
                    fout << "  W " << weight << "\n";
                }
                fout << "  B " << neuron.get_bias() << "\n";
            }
        }

        cout << "Model saved.\n";
        return;
    }

    cout << "ERROR: No free save slots available (0–29 used).\n";
    cout << "Delete some models to free up space";
}

Network Network::load_model(string path) {
    ifstream fin(path);
    if (!fin.is_open()) {
        cout << "ERROR: Could not open file '" << path << "'.\n";
        terminate();
    }
    Network network = Network(1);

    while (!fin.eof()) {
        string line;
        fin >> line;

        while (line == "LAYER") {
            Layer layer = Layer();
            fin >> line;
            while (line == "NEURON") {
                Neuron neuron = Neuron();
                fin >> line;
                while (line == "W") {
                    fin >> line;
                    float val = static_cast<float>(atof(line.c_str()));
                    neuron.add_weight(val);
                    fin >> line;
                }
                if (line == "B") {
                    fin >> line;
                    neuron.set_bias(static_cast<float>(atof(line.c_str())));
                }
                fin >> line;
                layer.add_neuron(neuron);
            }
            //fin >> line;
            network.add_layer(&layer, &network);
        }
    }
    fin.close();
    cout << "Model... Loaded? saving it to test..." << endl;
    return network;
}

void Network::add_layer(Layer* layer, Network* network) {
    network->layers.push_back(*layer);
}



Network& Network::operator=(const Network& other) {
    if (this != &other) {
        scoped_lock lock(mtx, other.mtx);
        layers = other.layers;
        last_input = other.last_input; // include if applicable
    }
    return *this;
}

void Network::copy_from(const Network& other) {
    scoped_lock lock(mtx, other.mtx);
    layers = other.layers;
    last_input = other.last_input; // optional, same reasoning
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
            }
        }
    }
}


vector<float> Network::forward(const vector<float>& input) {
    lock_guard <mutex> lock(mtx);

    last_input = input;
    vector<float> activations = input;
    LeakyReLU act;

    for (size_t k = 0; k < layers.size(); ++k) {
        Layer& layer = layers[k];
        vector<float> next(layer.get_layer_size());

        for (size_t i = 0; i < layer.get_layer_size(); ++i) {
            Neuron& neuron = layer.get_neurons()[i];
            const vector<float>& weights = neuron.get_weights();
            float z = neuron.get_bias();

            for (size_t j = 0; j < weights.size(); ++j)
                z += weights[j] * activations[j];

            float activated = (k < layers.size() - 1)
                ? act.activate(z)
                : z;

            neuron.z = z;
            neuron.a = activated;
            next[i] = activated;
        }

        activations.swap(next);
    }

    return activations;
}


void Network::backward(const vector<float>& target) {
    lock_guard <mutex> lock(mtx);


    LeakyReLU act;

    // Output layer
    Layer& outputLayer = layers.back();
    for (size_t j = 0; j < outputLayer.get_layer_size(); ++j) {
        Neuron& neuron = outputLayer.get_neurons()[j];


        float error = neuron.a - target[j];

        if (error > 1.0f) error = 1.0f;
        if (error < -1.0f) error = -1.0f;

        neuron.delta = error;
    }

    // Hidden layers
    for (int l = static_cast<int>(layers.size()) - 2; l >= 0; --l) {
        Layer& current = layers[l];
        Layer& next = layers[l + 1];

        for (size_t j = 0; j < current.get_layer_size(); ++j) {
            float sum = 0.0f;
            for (size_t k = 0; k < next.get_layer_size(); ++k)
                sum += next.get_neurons()[k].get_weights()[j] * next.get_neurons()[k].delta;

            current.get_neurons()[j].delta = sum * act.derivative(current.get_neurons()[j].z);
        }
    }

    // Weight updates
    for (size_t l = 0; l < layers.size(); ++l) {
        vector<float> prev_activations = (l == 0)
            ? last_input
            : layers[l - 1].get_activations();

        for (Neuron& neuron : layers[l].get_neurons()) {
            auto& weights = neuron.get_weights();
            for (size_t w = 0; w < weights.size(); ++w)
                weights[w] -= learning_rate_config * neuron.delta * prev_activations[w];

            neuron.set_bias(neuron.get_bias() - learning_rate_config * neuron.delta);
        }
    }
}







