//
// Created by edward on 10/15/25.
//

#ifndef FLAPPYBIRD_NUERON_H
#define FLAPPYBIRD_NUERON_H
#include <vector>


using namespace std;


class Neuron {
private:
    std::vector<float> weights;
    float bias;
    float learning_rate;

public:
    float z;      // pre-activation sum
    float a;      // output after activation
    float delta;  // backpropagated error

    explicit Neuron(int weights_size);
    Neuron(const Neuron& other);

    Neuron &operator=(const Neuron &other);

    void set_weights();

    float get_bias() const;

    void set_bias(float val);


    std::vector<float>& get_weights();
};



#endif //FLAPPYBIRD_NUERON_H