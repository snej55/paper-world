#ifndef PLAYER_H
#define PLAYER_H

#include "SDL2/SDL.h"

#include "./util.hpp"
#include "./vec2.hpp"
#include "./tiles.hpp"
#include "./particles.hpp"
#include "./sparks.hpp"
#include "./anim.hpp"
#include "./weapons.hpp"
#include "./shockwaves.hpp"

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
    const SDL_Color _Palette[5] {{0xa8, 0x60, 0x5d}, {0xd1, 0xa6, 0x7e}, {0xf6, 0xe7, 0x9c}, {0x65, 0x40, 0x53}, {0x1f, 0x24, 0x4b}};
    vec2<double> _pos;
    vec2<double> _last_pos{0.0, 0.0};
    vec2<int> _dimensions;
    vec2<double> _spawn_pos;

    vec2<double> _vel{0.0, 0.0};
    double _friction {0.48};
    double _gravity {0.3};

    double _falling {99.9};

    SDL_Rect _rect {0, 0, 1, 1};
    Controller _Controller{};

    double _ad {100};
    double _death_time{120};

    ParticleSpawner _Particles{10000, 0, {50.0, 50.0}, {1.0, 1.0}, 0.125, 0.01, true};
    SmokeSpawner _Smoke{10000, 0, {100.0, 100.0}, 0.15, true};
    FireSpawner _Fire{10000, 0, {200.0, 200.0}, 0.2, true};
    SparkManager _SparkManager{0.0, 0.2, 1.0, nullptr};

    double _max_health{100.0};
    double _health{100.0};
    double _recover{100.0}; // times since damaged
    double _recover_time{10.0};

    // Animations
    Anim* _idleAnim;
    Anim* _runAnim;
    Anim* _jumpAnim;
    Anim* _landAnim;
    Texture* _flash;
    Anim* _anim {nullptr}; // animation we render;

    double _grounded{99.9};
    bool _flipped{false};

    // -------- Weapons -------- //
 
    // for the sword
    bool _swordAttacking {false};
    double _swordAttacked {0};
    bool _slashVFLIP {false};
    Slash* _Slash{nullptr};
    Sword* _Sword;

    bool _should_damage{false};
    double _in_water{150.0};
    bool _lava_struck{false};

public:
    Player(vec2<double> pos, vec2<int> dimensions);
    ~Player();

    void free();

    void reset();

    void loadAnim(TexMan* texman);

    bool getLavaStruck() {return _lava_struck;}
    void setLavaStruck(bool val) {_lava_struck = val;}

    vec2<double> getLastPos() {return _last_pos;}

    SDL_Rect* getRect();
    void updateRect()
    {
        _rect.x = _pos.x;
        _rect.y = _pos.y;
    }
    SDL_Rect getAttackRect();
    bool getAttacking();
    double getSwordDamage() const 
    {
        if (_Sword != nullptr)
        {
            return _Sword->getDamage();
        }
        return 0.0;
    }

    bool getShouldDamage()
    {
        return _should_damage;
    }

    void setSpawnPos(vec2<double> pos) {_pos = pos; _spawn_pos = pos;}

    double getInWater() {return _in_water;}
    void setInWater(double val) {_in_water = val;}

    double getFalling() {return _falling;}
    void setVelY(const double val) {_vel.y = val;}
    void setVelX(const double val) {_vel.x = val;}
    double getVelY() {return _vel.y;}
    double getVelX() {return _vel.x;}

    Controller* getController();
    // Sword* getSword() {return _Sword;}
    vec2<double>& getPos();
    vec2<double> getCenter();
    bool getFlipped() {return _flipped;}

    double getHealth() {return _health;}
    void setHealth(double val) {_health = val;}
    double getMaxHealth() {return _max_health;}
    double getRecover() {return _recover;}
    void setRecover(double val) {_recover = val;}

    void damage(double amount, double* screen_shake, double* slomo, ShockWaveManager& shockwaves);

    double getAd() {return _ad;}
    void setAd(double val) {_ad = val;}
    void tickAd(const double& time_step)
    {
        if (_ad < 60.0)
        {
            _Smoke.setSpawning(std::max(_Smoke.getSpawning(), 2), {1, 2}, {0x88, 0x88, 0xBB});
        }
        if (_ad < 20.0)
        {
            _Fire.setSpawning(std::max(_Fire.getSpawning(), 10));
        }
        if (_ad < 2 && _ad > 0)
        {
            _Particles.setSpawning(48, {1.0, 20.0}, _Palette[0]);
        }
        if (_lava_struck)
        {
            if (_ad < 110.0)
            {
                _Fire.setSpawning(std::max(_Fire.getSpawning(), 10));
                _Smoke.setSpawning(std::max(_Smoke.getSpawning(), 2), {1, 2}, {0x88, 0x88, 0xBB});
            }
        }
        if (_ad + time_step > _death_time)
        {
            _lava_struck = false;
        }
        _ad += time_step;
        _recover += time_step;
    }

    void die(double* screen_shake, ShockWaveManager& shockwaves);

    void update(const double& time_step, World& world, double* screen_shake, TexMan* texman, ShockWaveManager& shockwaves);
    void updateVel(const double& time_step);
    void handlePhysics(const double& time_step, vec2<double> frame_movement, World& world, double* screen_shake, TexMan* texman, ShockWaveManager& shockwaves);
    void updateParticles(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, TexMan* tex);

    void handleAnim(const double& time_step);

    void render(const int scrollX, const int scrollY, SDL_Renderer* renderer);
    void renderPlayer(const int scrollX, const int scrollY, SDL_Renderer* renderer);
    void renderSword(const int scrollX, const int scrollY, SDL_Renderer* renderer);
    void renderSlash(const int scrollX, const int scrollY, SDL_Renderer* renderer);

    // void updateSword(const double& time_step);
    // void renderSword(const int scrollX, const int scrollY, SDL_Renderer* renderer, TexMan* texman);
    void attackSword(TexMan* texman);
};

#endif