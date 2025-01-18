#ifndef ANIM_H
#define ANIM_H

#include "./texture.hpp"
#include <cmath>

class Anim
{
private:
    SDL_RendererFlip _flipped {SDL_FLIP_NONE};

    double _angle{0};

    const int _width;
    const int _height;

    const int _length;
    double _frame{0.0};
    double _speed;
    bool _loop;

    bool _finished{false};

    int _step{0};

    Texture* _texture;

public:
    Anim(int w, int h, int length, double speed, bool loop, Texture* texture);
    ~Anim();

    void setTex(Texture* texture);

    SDL_RendererFlip getFlipped() {return _flipped;}
    void setFlipped(SDL_RendererFlip flip) {_flipped = flip;}
    bool getFinished() {return _finished;}
    void setFinished(bool val) {_finished = val;}
    void setAngle(double val) {_angle = val;}
    double getAngle() {return _angle;}
    void reset();

    double getFrame() {return _frame;}

    void setFrame(double frame)
    {
        _frame = frame;
        _step = static_cast<int>(_frame) % _length;
    }

    // update
    void tick(const double& time_step);

    // render
    void render(int x, int y, const int scrollX, const int scrollY, SDL_Renderer* renderer);
};

#endif