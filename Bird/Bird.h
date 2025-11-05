//
// Created by edward on 10/8/25.
//

#ifndef FLAPPYBIRD_BIRD_H
#define FLAPPYBIRD_BIRD_H


class Bird {
    public:
        float gravity;
        float x;
        float y;
        float width;
        float height;
        float y_vel;
        int score;
        bool unchecked_score;

    Bird();
    void update();
    //bool check_collision();
};


#endif //FLAPPYBIRD_BIRD_H