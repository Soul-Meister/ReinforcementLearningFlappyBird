//
// Created by edward on 10/8/25.
//

#ifndef FLAPPYBIRD_BIRD_H
#define FLAPPYBIRD_BIRD_H


class Bird {
    public:
        double gravity = 5;
        double x = 390;
        double y = 220;
        double width = 20;
        double height = 20;
        double y_vel = 0;

    Bird();
    void update();
    //bool check_collision();
};


#endif //FLAPPYBIRD_BIRD_H