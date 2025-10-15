//
// Created by edward on 10/8/25.
//

#ifndef FLAPPYBIRD_BIRD_H
#define FLAPPYBIRD_BIRD_H


class Bird {
    public:
        double gravity;
        double x;
        double y;
        double width;
        double height;
        double y_vel;
        int score;

    Bird();
    void update();
    //bool check_collision();
};


#endif //FLAPPYBIRD_BIRD_H