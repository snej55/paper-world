#ifndef PLAYER_H
#define PLAYER_H

#include <SDL2/SDL.h>

#include "./util.hpp"
#include "./vec2.hpp"
#include "./tiles.hpp"

enum class Control
{
    UP,
    DOWN,
    LEFT,
    RIGHT,
};

class Controller
{
private:
    bool _control_up{false};
    bool _control_down{false};
    bool _control_left{false};
    bool _control_right{false};

    float _jumping {99.0};

public:
    void setControl(Control control, const bool val);

    bool const getControl(Control control);

    void setJumping(float val) {_jumping = val;}
    float const getJumping() {return _jumping;}

    void update(const double& time_step);
};

class Player
{
private:
    vec2<double> _pos;
    vec2<int> _dimensions;
    vec2<double> _spawn_pos;

    vec2<double> _vel{0.0, 0.0};
    double _friction {0.48};
    double _gravity {0.3};

    double _falling {99.9};

    SDL_Rect _rect {0.0, 0.0, 1.0, 1.0};
    Controller _Controller{};

    double _ad {100};
    double _death_time{120};

public:
    Player(vec2<double> pos, vec2<int> dimensions);

    SDL_Rect* getRect();
    Controller* getController();
    vec2<double>& getPos();
    double getAd() {return _ad;}
    void tickAd(const double& time_step) {_ad += time_step;}

    void update(const double& time_step, World& world);
    void updateVel(const double& time_step);
    void handlePhysics(const double& time_step, vec2<double> frame_movement, World& world);

    void render(const int scrollX, const int scrollY, SDL_Renderer* renderer);
};

#endif