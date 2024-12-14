#ifndef SWORD_H
#define SWORD_H

#include "./anim.hpp"
#include "./player.hpp"

// class Slash
// {
// private:
//     Anim* _Slash;
//     vec2<double> _pos;
//     SDL_RendererFlip _flipped{SDL_FLIP_NONE};

// public:
//     Slash(TexMan* texman);
//     ~Slash();

//     void draw(const int scrollX, const int scrollY, SDL_Renderer* renderer, vec2<double> target_pos = {0, 0});
// };

class Sword
{
private:
    vec2<double> _pos;
    Player* _Player;
    double _angle{0};
    vec2<double> _offset;
    bool _attacking{false};
    double _angle_offset{90.0};
    double _swing_dir{0.0};
    double _swing_vel{0.0};
    const double _arm_length{1.5};
    double _attacked{10.0};
    double _target_turn{180.0};
    SDL_RendererFlip _flipped{SDL_FLIP_NONE};
    double _target_dir{M_PI};
    double _damp{0.5}; // spring dampening

public:
    Sword(vec2<double> pos, Player* player);
    
    void attack();
    void update(const double& time_step);

    void render(const int scrollX, const int scrollY, SDL_Renderer* renderer);
}

#endif