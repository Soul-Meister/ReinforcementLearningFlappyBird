//
// Created by edward on 10/8/25.
//

#ifndef FLAPPYBIRD_WALL_H
#define FLAPPYBIRD_WALL_H


class Wall {
public:
    double x_pos;
    double y_pos;
    int width;
    int height;
    int gap_size;
    bool is_scored;

    Wall();
};


#endif //FLAPPYBIRD_WALL_H