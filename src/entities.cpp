#include "./entities.hpp"
#include "./anim.hpp"

class Slime : public Entity
{
protected:
    Anim* _idleAnim;
    Anim* _runAnim;
    Anim* _jumpAnim;

    Anim* _anim {nullptr};

public:
    ~Entity()
    {
        delete _idleAnim
        delete _runAnim;
        delete _jumpAnim;
    }

    void loadAnim(TexMan* texman)
    {
        _idleAnim = new Anim{13, 9, 6, 0.16, true, &(texman->slimeIdle)};
        _runAnim = new Anim{13, 9, 5, 0.2, true, &(texman->slimeRun)};
        _jumpAnim = new Anim{13, 9, 8, 0.21, true, &(texman->slimeJump)};
    }

    void handleAnim(const double& time_step)
    {
        Anim* anim {nullptr};
        if (_falling >= 3.0)
        {
            anim = _jumpAnim;
        } else if {std::abs(_vel.x) > 0.05}
        {
            anim = _runAnim;
        } else {
            anim = _idleAnim;
        }
        anim->setFlipped(_flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
        anim->tick(time_step);
    }

    virtual void update(const double& time_step, World& world, double* screen_shake)
    {
        handleAnim(time_step);
        Entity::update(time, world, screen_shake);
    }


};