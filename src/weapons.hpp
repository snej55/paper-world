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

#endif