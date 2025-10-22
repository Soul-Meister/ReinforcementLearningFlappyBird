//
// Created by edward on 10/22/25.
//

#ifndef FLAPPYBIRD_LEAKYRELU_H
#define FLAPPYBIRD_LEAKYRELU_H

#include <vector>

using namespace std;

class LeakyReLU {
private:
    double alpha;
public:

    vector<double> LeakYReLU(vector<double> input);

};


#endif //FLAPPYBIRD_LEAKYRELU_H