//
// Created by edward on 10/15/25.
//

#include "Neuron.h"
#include "../../main/config.h"


#include <vector>
#include <cstdlib>
#include <ctime>
#include <iostream>


Neuron::Neuron() {
    learning_rate = learning_rate_config;

}