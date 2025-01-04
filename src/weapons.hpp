#ifndef WEAPONS_H
#define WEAPONS_H

#include "./vec2.hpp"
#include "./texman.hpp"
#include "./anim.hpp"

// -------------------------------------------------- Slash -------------------------------------------------- //

class Slash
{
private:
    bool _vflip; // flipped vertically?
    bool _flip; // flipped horizontally?
    vec2<double> _offset;
    Anim* _anim;
    bool _finished{false};

public:
    Slash(bool vflip, bool flip, vec2<double> offset, TexMan* texman);
    ~Slash();

    bool getFinished() {return _finished;}

    void loadAnim(TexMan* texman);

    void update(const double& time_step);
    void render(const int scrollX, const int scrollY, SDL_Renderer* renderer, void* target);
};

// -------------------------------------------------- Sword -------------------------------------------------- //

class Sword
{
private:
    void* _Player;
    bool _flipped{false};

    vec2<double> _pos{0, 0};

    // turning stuff
    double _angle{0.0};
    double _turn{0.0};
    const double _arm_length{2.5};
    double _target_angle{0.0};
    double _target_turn{0.0};
    double _angle_vel{0.0};

    bool _up{true};

    // rendering
    Texture* _tex;

    const double _damage{2.0};

public:
    Sword(void* Player, TexMan* texman);

    void loadTex(TexMan* texman);

    const double getDamage() const {return _damage;}

    bool getUp() {return _up;}
    void setUp(bool val) {_up = val;}
    void flipUp() {_up = !_up;}
    
    void update(const double& time_step);
    void render(const int scrollX, const int scrollY, SDL_Renderer* renderer);
};

#endif
