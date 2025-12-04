//
// Created by edward on 10/15/25.
//

#ifndef FLAPPYBIRD_NETWORK_H
#define FLAPPYBIRD_NETWORK_H

#include <mutex>
#include <string>
#include <vector>
#include "../Layer/Layer.h"

class Network {
private:
    vector<float> last_input;
    mutable std::mutex mtx;
public:
    vector<Layer> layers;

    Network(int mode);

    void save_model(Network*);
    static Network load_model(string);

    Network(const Network &other);

    Network &operator=(const Network &other);

    void copy_from(const Network &other);

    void init();
    vector<float> forward(const vector<float> &input);
    void backward(const vector<float>& target);
    int get_layer_num();
    static void add_layer(Layer* layer, Network* network);
};


#endif //FLAPPYBIRD_NETWORK_H