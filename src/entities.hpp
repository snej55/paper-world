#ifndef ENTITIES_H
#define ENTITIES_H

#include "SDL2/SDL.h"
#include "JSON/json.hpp"

#include <string>
#include <fstream>

#include "./util.hpp"
#include "./vec2.hpp"
#include "./tiles.hpp"
#include "./player.hpp"
#include "./anim.hpp"
#include "./health_bars.hpp"
#include "./sparks.hpp"

class Entity
{
protected:
    static inline int sId{0};
    vec2<double> _pos;
    vec2<double> _vel;
    vec2<int> _dimensions {8, 8};
    double _gravity;
    bool _peaceful;
    std::string _name;

    vec2<int> _anim_offset{0, 0};

    EntityHealthBar* _health_bar {nullptr};

    double _falling{99.0};
    double _top_speed{1.0};

    SDL_Rect _rect{0, 0, 0, 0};
    bool _should_die {false};
    bool _should_damage{false};

    double _maxHealth{10.0};
    double _health{10.0};
    double _damage{5.0};
    double _recover{100.0};
    double _recover_time{10.0};

    bool _flipped{false}; // flipping for moving direction
    bool _wandering{false}; // if it is moving while it is wandering
    double _wander_timer{static_cast<double>(std::rand() % 180)};
    bool _anim_flipped{false}; // flipped for animation

    int _id{0};
    // palette length must be 8 (don't ask)
    const SDL_Color _Palette[8] {{0xa8, 0x60, 0x5d}, {0xd1, 0xa6, 0x7e}, {0xf6, 0xe7, 0x9c}, {0xb6, 0xcf, 0x8e}, {0x60, 0xae, 0x7b}, {0x3c, 0x6b, 0x64}, {0x1f, 0x24, 0x4b}, {0x65, 0x40, 0x53}};

public:
    Entity(vec2<double> pos, vec2<double> vel, double gravity, bool peaceful, std::string name);
    virtual ~Entity()
    {
        if (_health_bar != nullptr)
        {
            delete _health_bar;
        }
    }

    virtual void loadAnim(TexMan* texman)
    {
        _health_bar = new EntityHealthBar{&(texman->enemyHealthBar), {12, 2}, getMaxHealth()};
        _health_bar->setDimensions({_dimensions.x, 2});
    }

    int getId();
    virtual double getHealth() {return _health;}
    virtual double getMaxHealth() {return _maxHealth;}
    virtual void setHealth(double val) {_health = val;} 
    const vec2<int> getAnimOffset() const {return _anim_offset;}

    vec2<double>& getPos();
    vec2<double> getCenter();
    void updateRect();
    SDL_Rect* getRect();
    bool getShouldDie();
    bool getShouldDamage() {return _should_damage;}
    void setShouldDamage(bool val) {_should_damage = val;}
    bool getFlipped();
    bool getPeaceful();
    std::string_view getName();

    virtual void setPalette(ParticleSpawner* particles)
    {
        particles->setPalette<8>(_Palette);
    }

    virtual void damage(const double damage, double* screen_shake);
    virtual void die(double* screen_shake);

    virtual void update(const double& time_step, World& world, double* screen_shake);

    virtual void updateVel(const double& time_step);

    virtual void handlePhysics(const double& time_step, vec2<double> frame_movement, World& world, double* screen_shake);

    virtual void render(const int scrollX, const int scrollY, SDL_Renderer* renderer);

    virtual void touchPlayer(Player* player, double* screen_shake, double* slomo);

    virtual void followPlayer(Player* player, World* world, const double& time_step);

    virtual void wander(World* world, const double& time_step);

    void updateHealthBar();
    void renderHealthBar(const int scrollX, const int scrollY, SDL_Renderer* renderer);
};

class Slime : public Entity
{
protected:
    Anim* _idleAnim;
    Anim* _runAnim;
    Anim* _jumpAnim;
    Anim* _flash;

    Anim* _anim {nullptr};

    vec2<int> _dimensions{11, 7};
    vec2<int> _anim_offset{1, 1};

    double _maxHealth{40.0};
    double _health{40.0};
    double _damage{10.0};

    const SDL_Color _Palette[8] {{0x3c, 0x6b, 0x64}, {0xf6, 0xe7, 0x9c}, {0x60, 0xae, 0x7b}, {0x1f, 0x24, 0x4b}, {0x3c, 0x6b, 0x64}, {0xf6, 0xe7, 0x9c}, {0x60, 0xae, 0x7b}, {0x1f, 0x24, 0x4b}};

public:
    Slime(vec2<double> pos, vec2<double> vel, double gravity, bool peaceful, std::string name, TexMan* texman);

    virtual ~Slime();

    virtual double getHealth() {return _health;}
    virtual double getMaxHealth() {return _maxHealth;}
    virtual void setHealth(double val) {_health = val;}

    void loadAnim(TexMan* texman);

    void handleAnim(const double& time_step);

    virtual void update(const double& time_step, World& world, double* screen_shake);

    virtual void render(const int scrollX, const int scrollY, SDL_Renderer* renderer);
};

class Bat : public Entity
{
private:
    Anim* _anim;
    Anim* _flash;
    Texture* _glowTex;
    vec2<int> _dimensions{3, 4};
    vec2<int> _anim_offset{2, 0};

    double _maxHealth{20.0};
    double _health{20.0};
    double _damage{3.0};

    double _angle{0.0};
    double _speed{Util::random() * 1.0 + 0.25};

    const SDL_Color _Palette[8] {{0xa8, 0x60, 0x5d}, {0x65, 0x40, 0x53}, {0x1f, 0x24, 0x4b}, {0xa8, 0x60, 0x5d}, {0x65, 0x40, 0x53}, {0x1f, 0x24, 0x4b}, {0xa8, 0x60, 0x5d}, {0x65, 0x40, 0x53}};

public:
    Bat(vec2<double> pos, vec2<double> vel, double gravity, bool peaceful, std::string name, TexMan* texman);

    virtual ~Bat();

    virtual double getHealth() {return _health;}
    virtual double getMaxHealth() {return _maxHealth;}
    virtual void setHealth(double val) {_health = val;}

    void loadAnim(TexMan* texman);

    void handleAnim(const double& time_step);

    virtual void damage(const double damage, double* screen_shake);

    virtual void handlePhysics(const double& time_step, vec2<double> frame_movement, World& world, double* screen_shake);

    virtual void touchPlayer(Player* player, double* screen_shake, double* slomo);

    virtual void followPlayer(Player* player, World* world, const double& time_step);

    virtual void update(const double& time_step, World& world, double* screen_shake);

    virtual void render(const int scrollX, const int scrollY, SDL_Renderer* renderer);
};

class Turtle : public Entity
{
private:
    Anim* _idleAnim;
    Anim* _runAnim;
    Anim* _jumpAnim;
    Anim* _landAnim;
    Anim* _flash;
    Anim* _anim;

    vec2<int> _dimensions{8, 8};
    vec2<int> _anim_offset{0, 0};

    double _maxHealth{60.0};
    double _health{60.0};
    double _damage{4.0};

    double _grounded{0.0};

public:
    Turtle(vec2<double> pos, vec2<double> vel, double gravity, bool peaceful, std::string name, TexMan* texman);

    virtual ~Turtle();

    virtual double getHealth() {return _health;}
    virtual double getMaxHealth() {return _maxHealth;}
    virtual void setHealth(double val) {_health = val;}

    void loadAnim(TexMan* texman);

    void handleAnim(const double& time_step);

    void setGrounded(const double grounded)
    {
        _grounded = grounded;
    }

    virtual void damage(const double damage, double* screen_shake);

    virtual void touchPlayer(Player* player, double* screen_shake, double* slomo);

    virtual void wander(World* world, const double& time_step);

    virtual void update(const double& time_step, World& world, double* screen_shake);

    virtual void render(const int scrollX, const int scrollY, SDL_Renderer* renderer);
};

class EntityManager
{
private:
    int _total;
    Entity** _Entities;

    vec2<double> _pos;
    std::string _name;

    ParticleSpawner _Particles{10000, 0, {50.0, 50.0}, {1.0, 1.0}, 0.125, 0.01, true};
    SmokeSpawner _Smoke{10000, 0, {100.0, 100.0}, 0.15, true};
    FireSpawner _Fire{10000, 0, {200.0, 200.0}, 0.2, true};

    SparkManager _SparkManager{0.0, 0.2, 1.0, nullptr};

public:
    EntityManager(vec2<double> pos, const int total, Entity** entities);

    EntityManager(vec2<double> pos, const int total, std::vector<Entity*> entities);

    ~EntityManager();

    void free();

    std::string_view getName();

    Entity* getEntity(std::size_t idx);

    int getTotal();

    virtual void addEntity(Entity* entity);

    virtual void update(const double& time_step, World& world, double* screen_shake, Player* player, double* slomo, TexMan* texman);

    virtual void updateParticles(const double& time_step, const int scrollX, const int scrollY, SDL_Renderer* renderer, World* world, TexMan* texman);

    virtual void render(const int scrollX, const int scrollY, SDL_Renderer* renderer);
};

// "Manager of the Managers" Entity-Manager-Manager
class EMManager
{
private:
    std::vector<EntityManager*> _Managers;
    // entities will be std::vector<std::vector<Entity*>> entities; in void loadEntities();

public:
    EMManager();

    ~EMManager();

    void free();

    void loadFromPath(std::string path, TexMan* texman);

    void addEntity(Entity* entity);

    void update(const double& time_step, World& world, double* screen_shake, Player* player, double* slomo, TexMan* texman);

    void render(const int scrollX, const int scrollY, SDL_Renderer* renderer, const double& time_step, World* world, TexMan* texman);
};

#endif