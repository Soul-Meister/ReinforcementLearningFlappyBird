//
// Created by edward on 10/22/25.
//

#include "LeakyReLU.h"
#include "../../main/Config.h"

LeakyReLU::LeakyReLU() {
    alpha = alpha_config;
}

float LeakyReLU::activate(float input) {
    return (input > 0.0f) ? input : static_cast<float>(alpha * input);
}

float LeakyReLU::derivative(float input) {
    return (input > 0.0f) ? 1.0f : static_cast<float>(alpha);
}
