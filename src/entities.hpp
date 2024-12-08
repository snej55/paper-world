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
    Slime(vec2<double> pos, vec2<double> vel, double gravity, bool peaceful, std::string name, TexMan* texman)
     : Entity{pos, vel, gravity, peaceful, name}
    {
        loadAnim(texman);
    }

    ~Slime()
    {
        delete _idleAnim;
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
        _anim = _idleAnim;
        if (_falling >= 3.0)
        {
            _anim = _jumpAnim;
            _runAnim->reset();
            _idleAnim->reset();
        } else if (std::abs(_vel.x) > 0.05)
        {
            _anim = _runAnim;
            _jumpAnim->reset();
            _idleAnim->reset();
        } else {
            _anim = _idleAnim;
            _jumpAnim->reset();
            _runAnim->reset();
        }
        _anim->setFlipped(_anim_flipped ? SDL_FLIP_HORIZONTAL : SDL_FLIP_NONE);
        _anim->tick(time_step);
    }

    virtual void update(const double& time_step, World& world, double* screen_shake)
    {
        handleAnim(time_step);
        Entity::update(time_step, world, screen_shake);
    }

    virtual void render(const int scrollX, const int scrollY, SDL_Renderer* renderer)
    {
        _anim->render((int)_pos.x - _anim_offset.x, (int)_pos.y - _anim_offset.y, scrollX, scrollY, renderer);
    }
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
    Bat(vec2<double> pos, vec2<double> vel, double gravity, bool peaceful, std::string name, TexMan* texman)
     : Entity{pos, vel, 0.05, peaceful, name}
    {
        loadAnim(texman);
    }

    ~Bat()
    {
        delete _anim;
    }

    void loadAnim(TexMan* texman)
    {
        _anim = new Anim{7, 4, 2, 0.3, true, &(texman->bat)};
        _glowTex = &(texman->lightTex);
    }

    void handleAnim(const double& time_step)
    {
        _anim->setAngle(_angle);
        _anim->tick(time_step);
    }

    virtual void handlePhysics(const double& time_step, vec2<double> frame_movement, World& world, double* screen_shake)
    {
        _pos.x += frame_movement.x * time_step * _speed;
        _rect.x = _pos.x;
        _rect.y = _pos.y;

        std::array<SDL_Rect, 9> rects;
        world.getTilesAroundPos(_pos, rects);
        for (int i{0}; i < 9; ++i)
        {
            SDL_Rect* tile_rect {&(rects[i])};
            if (Util::checkCollision(&_rect, tile_rect))
            {
                if (frame_movement.x > 0)
                {
                    _rect.x = tile_rect->x - _rect.w;
                } else {
                    _rect.x = tile_rect->x + tile_rect->w;
                }
                _pos.x = _rect.x;
                _vel.x *= -1;
                _vel.y *= 1.2;
            }
        }

        _pos.y += frame_movement.y * time_step * _speed;
        _rect.x = _pos.x;
        _rect.y = _pos.y;

        world.getTilesAroundPos(_pos, rects);
        for (int i{0}; i < 9; ++i)
        {
            SDL_Rect* tile_rect {&(rects[i])};
            if (Util::checkCollision(&_rect, tile_rect))
            {
                if (frame_movement.y > 0)
                {
                    _rect.y = tile_rect->y - _rect.h;
                    _falling = 0.0;
                } else {
                    _rect.y = tile_rect->y + tile_rect->h;
                }
                _vel.y *= -1;
                _pos.y = _rect.y;
            }
        }

        world.getDangerAroundPos(_pos, rects);
        for (int i{0}; i < 9; ++i)
        {
            SDL_Rect* tile_rect{&(rects[i])};
            if (Util::checkCollision(&_rect, tile_rect))
            {
                die(screen_shake);
                break;
            }
        }
    }

    virtual void touchPlayer(Player* player, double* screen_shake)
    {
        _rect.x = _pos.x;
        _rect.y = _pos.y;
        SDL_Rect* player_rect{player->getRect()};
        if (Util::checkCollision(player_rect, &_rect))
        {
            double angle = Util::random() * 2.0 * M_PI;
            _vel.x += std::cos(angle) * 5.0;
            _vel.y += std::sin(angle) * 5.0;
            player->damage(_damage, screen_shake);
        }
    }

    virtual void followPlayer(Player* player, World* world)
    {
        SDL_Rect* player_rect{player->getRect()};
        vec2<double> player_pos{player->getCenter()};
        if (Util::distance(player_pos, getCenter()) < 200.0)
        {
            _vel.x += std::max(-0.1, std::min(0.1, (player_pos.x - getCenter().x) * 0.005));
            _vel.y += std::max(-0.1, std::min(0.1, (player_pos.y - getCenter().y) * 0.005));
            vec2<double> check_pos {getCenter().x + _vel.x * 2.0, getCenter().y + _vel.y * 2.0};
            Tile* tile {world->getTileAt(check_pos.x, check_pos.y)};
            if (tile != nullptr)
            {    
                if (Util::elementIn<TileType, std::size(DANGER_TILES)>(tile->type, DANGER_TILES))
                {
                    _vel.x *= -1;
                    _vel.y *= -1;
                }
            }
            _vel.x *= 0.95;
            _vel.y *= 0.95;
        } else {
            wander(world);
        }
    }

    virtual void update(const double& time_step, World& world, double* screen_shake)
    {
        handleAnim(time_step);
        Entity::update(time_step, world, screen_shake);
    }

    virtual void render(const int scrollX, const int scrollY, SDL_Renderer* renderer)
    {
        _glowTex->setBlendMode(SDL_BLENDMODE_ADD);
        _glowTex->setAlpha(10);
        _glowTex->setColor(246, 231, 156);
        SDL_Rect renderQuad{static_cast<int>(getCenter().x - 6 - _anim_offset.x) - scrollX, static_cast<int>(getCenter().y - 6 - _anim_offset.y - 2) - scrollY, 10, 10};
        SDL_RenderCopyEx(renderer, _glowTex->getTexture(), NULL, &renderQuad, 0, NULL, SDL_FLIP_NONE);
        //_glowTex->render(static_cast<int>(getCenter().x - 2.5 - _anim_offset.x) - scrollX, static_cast<int>(getCenter().y - 2.5 - _anim_offset.y - 2) - scrollY, renderer, &clip);
        _anim->render((int)_pos.x - _anim_offset.x, (int)_pos.y - _anim_offset.y, scrollX, scrollY, renderer);
    }
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
    EntityManager(vec2<double> pos, const int total, Entity** entities)
     : _total{total}, _pos{pos}
    {
        _Entities = new Entity*[total];
        for (std::size_t i{0}; i < total; ++i)
        {
            _Entities[i] = entities[i];
            _name = _Entities[i]->getName();
        }
    }

    EntityManager(vec2<double> pos, const int total, std::vector<Entity*> entities)
     : _total{total}, _pos{pos}
    {
        _Entities = new Entity*[total];
        for (std::size_t i{0}; i < total; ++i)
        {
            _Entities[i] = entities[i];
            _name = _Entities[i]->getName();
        }
    }

    ~EntityManager()
    {
        free();
    }

    void free()
    {
        for (std::size_t i{0}; i < _total; ++i)
        {
            delete _Entities[i];
        }
        delete _Entities;
    }

    std::string_view getName()
    {
        return _name;
    }

    Entity* getEntity(std::size_t idx) {return _Entities[idx];}

    int getTotal() {return _total;}

    virtual void addEntity(Entity* entity)
    {
        _Entities[_total] = entity;
        ++_total;
    }

    virtual void update(const double& time_step, World& world, double* screen_shake, Player* player)
    {
        const int num{_total};
        for (std::size_t i{0}; i < num; ++i)
        {
            if (i < _total)
            {
                Entity* entity {_Entities[i]};
                entity->update(time_step, world, screen_shake);
                if (!(entity->getPeaceful()))
                {
                    entity->followPlayer(player, &world);
                    entity->touchPlayer(player, screen_shake);
                } else {
                    entity->wander(&world);
                }
                // some black magic
                if (entity->getShouldDie())
                {
                    Util::swap(&_Entities[i], &_Entities[_total - 1]); // swap dead entity with last entity in the array
                    delete _Entities[_total - 1]; // deallocate dead entity
                    --_total; // deincrement total to avoid undefined behaviour when we reference nullptr
                }
            }
        }
        //std::cout << _total << '\n';
    }

    virtual void render(const int scrollX, const int scrollY, SDL_Renderer* renderer)
    {
        const int num{_total};
        for (std::size_t i{0}; i < num; ++i)
        {
            if (i < _total)
            {
                Entity* entity {_Entities[i]};
                entity->render(scrollX, scrollY, renderer);
            }
        }
    }
};

// "Manager of the Managers" Entity-Manager-Manager
class EMManager
{
private:
    std::vector<EntityManager*> _Managers;
    // entities will be std::vector<std::vector<Entity*>> entities; in void loadEntities();

public:
    EMManager()
    {
    }

    ~EMManager()
    {
        free();
    }

    void free()
    {
        for (std::size_t i{0}; i < _Managers.size(); ++i)
        {
            _Managers[i]->free();
            delete _Managers[i];
        }
        _Managers.clear();
    }

    void loadFromPath(std::string path, TexMan* texman)
    {
        std::ifstream f{path};
        json data = json::parse(f);

        std::vector<std::vector<Entity*>> entities;
        for (const auto& e : data["level"]["entities"])
        {
            std::string entity_name {e["type"]};
            bool found_entity{false};
            for (auto& entity_vec : entities)
            {
                if (!found_entity)
                {
                    for (Entity* entity : entity_vec)
                    {
                        if (entity->getName() == entity_name)
                        {
                            found_entity = true;
                            if (entity_name == "slime")
                            {
                                entity_vec.push_back(new Slime{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, false, entity_name, texman});
                            } else if (entity_name == "bat")
                            {
                                std::cout << "yo\n";
                                entity_vec.push_back(new Bat{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, false, entity_name, texman});
                            } else {
                                entity_vec.push_back(new Entity{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, false, "default"});
                            }
                            break;
                        }
                    }
                }
            }
            if (!found_entity)
            {
                if (entity_name == "slime")
                {
                    entities.push_back(std::vector<Entity*>{new Slime{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, false, entity_name, texman}});
                } else if (entity_name == "bat")
                {
                    std::cout << "yo\n";
                    entities.push_back(std::vector<Entity*>{new Bat{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, false, entity_name, texman}});
                } else {
                    entities.push_back(std::vector<Entity*>{new Entity{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, 0.2, false, "default"}});
                }
            }
        }
        
        for (EntityManager* manager : _Managers)
        {
            manager->free();
            delete manager;
        }
        _Managers.clear();

        for (auto& entity_vec : entities)
        {
            _Managers.push_back(new EntityManager{vec2<double>{0, 0}, entity_vec.size(), entity_vec});
        }

        f.close();
    }

    void addEntity(Entity* entity)
    {
        for (std::size_t i{0}; i < _Managers.size(); ++i)
        {
            if (_Managers[i]->getName() == entity->getName())
            {
                _Managers[i]->addEntity(entity);
                return; // break;
            }
        }
        // we didn't find anything
        _Managers.push_back(new EntityManager{vec2<double>{0, 0}, 1, std::vector<Entity*>{entity}});
    }

    void update(const double& time_step, World& world, double* screen_shake, Player* player)
    {
        for (std::size_t i{0}; i < _Managers.size(); ++i)
        {
            _Managers[i]->update(time_step, world, screen_shake, player);
        }
    }

    void render(const int scrollX, const int scrollY, SDL_Renderer* renderer)
    {
        for (std::size_t i{0}; i < _Managers.size(); ++i)
        {
            _Managers[i]->render(scrollX, scrollY, renderer);
        }
    }
};

#endif