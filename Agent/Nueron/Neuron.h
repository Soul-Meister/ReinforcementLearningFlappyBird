//
// Created by edward on 10/15/25.
//

#ifndef FLAPPYBIRD_NUERON_H
#define FLAPPYBIRD_NUERON_H
#include <vector>


class Neuron {

public:
    Neuron();
    double forward();
    void update_weights();
    double get_output();
    void print();
private:
    std::vector<double> weights;
    double bias;
    double output;
    double learning_rate;
};


#endif //FLAPPYBIRD_NEURON_H