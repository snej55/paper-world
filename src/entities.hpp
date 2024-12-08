#ifndef ENTITIES_H
#define ENTITIES_H

#include <SDL2/SDL.h>
#include <JSON/json.hpp>

#include <string>
#include <fstream>

#include "./util.hpp"
#include "./vec2.hpp"
#include "./tiles.hpp"
#include "./player.hpp"
#include "./anim.hpp"

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

    double _falling{99.0};
    double _top_speed{1.0};

    SDL_Rect _rect{0, 0, 0, 0};
    bool _should_die {false};

    double _health{10.0};
    double _damage{10.0};
    double _recover{100.0};
    double _recover_time{10.0};

    bool _flipped{false}; // flipping for moving direction
    bool _wandering{false}; // if it is moving while it is wandering
    double _wander_timer{0.0};
    bool _anim_flipped{false}; // flipped for animation

    int _id{0};

public:
    Entity(vec2<double> pos, vec2<double> vel, double gravity, bool peaceful, std::string name);

    int getId();

    vec2<double>& getPos();
    vec2<double> getCenter();
    void updateRect();
    SDL_Rect* getRect();
    bool getShouldDie();
    bool getFlipped();
    bool getPeaceful();
    std::string_view getName();

    virtual void die(double* screen_shake);

    virtual void update(const double& time_step, World& world, double* screen_shake);

    virtual void updateVel(const double& time_step);

    virtual void handlePhysics(const double& time_step, vec2<double> frame_movement, World& world, double* screen_shake);

    virtual void render(const int scrollX, const int scrollY, SDL_Renderer* renderer);

    virtual void touchPlayer(Player* player, double* screen_shake);

    virtual void followPlayer(Player* player, World* world);

    virtual void wander(World* world);
};

class Slime : public Entity
{
protected:
    Anim* _idleAnim;
    Anim* _runAnim;
    Anim* _jumpAnim;

    Anim* _anim {nullptr};

    vec2<int> _dimensions{11, 7};
    vec2<int> _anim_offset{1, 1};

public:
    Slime(vec2<double> pos, vec2<double> vel, double gravity, bool peaceful, std::string name, TexMan* texman);

    ~Slime();

    void loadAnim(TexMan* texman);

    void handleAnim(const double& time_step);

    virtual void update(const double& time_step, World& world, double* screen_shake);

    virtual void render(const int scrollX, const int scrollY, SDL_Renderer* renderer);
};

class Bat : public Entity
{
private:
    Anim* _anim;
    Texture* _glowTex;
    vec2<int> _dimensions{3, 4};
    vec2<int> _anim_offset{2, 0};

    double _angle{0.0};
    double _speed{Util::random() * 1.0 + 0.25};

public:
    Bat(vec2<double> pos, vec2<double> vel, double gravity, bool peaceful, std::string name, TexMan* texman);

    ~Bat();

    void loadAnim(TexMan* texman);

    void handleAnim(const double& time_step);

    virtual void handlePhysics(const double& time_step, vec2<double> frame_movement, World& world, double* screen_shake);

    virtual void touchPlayer(Player* player, double* screen_shake);

    virtual void followPlayer(Player* player, World* world);

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

public:
    EntityManager(vec2<double> pos, const int total, Entity** entities);

    EntityManager(vec2<double> pos, const int total, std::vector<Entity*> entities);

    ~EntityManager();

    void free();

    std::string_view getName();

    Entity* getEntity(std::size_t idx);

    int getTotal();

    virtual void addEntity(Entity* entity);

    virtual void update(const double& time_step, World& world, double* screen_shake, Player* player);

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

    void update(const double& time_step, World& world, double* screen_shake, Player* player);

    void render(const int scrollX, const int scrollY, SDL_Renderer* renderer);
};

#endif