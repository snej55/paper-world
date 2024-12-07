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

class Entity
{
private:
    vec2<double> _pos;
    vec2<double> _vel;
    vec2<int> _dimensions;
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

    bool _flipped{false};
    bool _wandering{false}; // if it is moving while it is wandering
    double _wander_timer{0.0};

public:
    Entity(vec2<double> pos, vec2<double> vel, vec2<int> dimensions, double gravity, bool peaceful, std::string name)
     : _pos{pos}, _vel{vel}, _dimensions{dimensions}, _gravity{gravity}, _peaceful{peaceful}, _name{name}
    { 
        _rect.x = pos.x;
        _rect.y = pos.y;
        _rect.w = dimensions.x;
        _rect.h = dimensions.y;
    }

    vec2<double>& getPos() {return _pos;}
    vec2<double> getCenter() {return {_pos.x + _dimensions.x / 2.0, _pos.y + _dimensions.y / 2.0};}
    SDL_Rect* getRect() {return &_rect;}
    bool getShouldDie() {return _should_die;}
    bool getFlipped() {return _flipped;}
    bool getPeaceful() {return _peaceful;}
    std::string_view getName() {return _name;}

    virtual void die(double* screen_shake) override
    {
        if (!_should_die)
            *screen_shake = std::max(*screen_shake, 8.0);
        _should_die = true;
        _vel.x = 0;
    }

    virtual void update(const double& time_step, World& world, double* screen_shake) override
    {
        _falling += time_step;
        _recover += time_step;
        _wander_timer -= time_step;
        if (_wander_timer <= 0.0)
        {
            _wandering = !_wandering;
            _wander_timer = (std::rand() % 180 )+ 60;
        }

        updateVel(time_step);
        handlePhysics(time_step, _vel, world, screen_shake);
    }

    virtual void updateVel(const double& time_step) override
    {
        _vel.x = std::min(std::max(_vel.x, -_top_speed), _top_speed);
        _vel.y += _gravity * time_step;
    }

    virtual void handlePhysics(const double& time_step, vec2<double> frame_movement, World& world, double* screen_shake) override
    {
        _pos.x += frame_movement.x * time_step;
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
                _vel.x = 0;
            }
        }

        _pos.y += frame_movement.y * time_step;
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
                _vel.y = 0.0;
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

    virtual void render(const int scrollX, const int scrollY, SDL_Renderer* renderer) override
    {
        SDL_Rect renderRect{_pos.x - scrollX, _pos.y - scrollY, _dimensions.x, _dimensions.y};
        if (_recover < _recover_time)
        {
            SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
        } else {
            SDL_SetRenderDrawColor(renderer, 0xFF, 0x00, 0x00, 0xFF);
        }

        SDL_RenderFillRect(renderer, &renderRect);
    }

    virtual void touchPlayer(Player* player, double* screen_shake) override
    {
        _rect.x = _pos.x;
        _rect.y = _pos.y;
        SDL_Rect* player_rect{player->getRect()};
        if (Util::checkCollision(player_rect, &_rect))
        {
            player->damage(_damage, screen_shake);
        }
    }

    virtual void followPlayer(Player* player, World* world) override
    {
        SDL_Rect* player_rect{player->getRect()};
        vec2<double> player_pos{player->getCenter()};
        if (Util::distance(player_pos, getCenter()) < 100.0)
        {
            vec2<double> checkTilePos {getCenter().x + (_flipped ? -10.0 : 10.0), getCenter().y + 8};
            Tile* tile {world->getTileAt(checkTilePos.x, checkTilePos.y)};
            if (tile == nullptr)
            {
                if (!(std::abs(static_cast<int>(player_pos.x - getCenter().x)) < 16 && player_pos.y > getCenter().y + 4.0 && _falling < 3.0))
                {
                    _vel.x += _flipped ? 0.2 : -0.2;
                    _flipped = !_flipped;
                }
            } else if (Util::elementIn<TileType, std::size(DANGER_TILES)>(tile->type, DANGER_TILES))
            {
                if (!(std::abs(static_cast<int>(player_pos.x - getCenter().x)) < 16 && player_pos.y > getCenter().y + 4.0 && _falling < 3.0))
                {
                    _vel.x += _flipped ? 0.2 : -0.2;
                    _flipped = !_flipped;
                }
            } else {
                if (player_pos.x > getCenter().x) // player is to the right of entity
                {
                    _flipped = false;
                } else {
                    _flipped = true;
                }
                _vel.x += _flipped ? -0.1 : 0.1;
            }
            if (player_pos.y + 4.0 < getCenter().y)
            {
                if (_falling < 3.0)
                {
                    _vel.y = -3.2;
                }
            }
        } else {
            wander(world);
        }
    }

    virtual void wander(World* world) override
    {
        if (_wandering)
        {
            vec2<double> checkTilePos {getCenter().x + (_flipped ? -10.0 : 10.0), getCenter().y + 8};
            Tile* tile {world->getTileAt(checkTilePos.x, checkTilePos.y)};
            if (tile == nullptr)
            {
                _vel.x += _flipped ? 0.2 : -0.2;
                _flipped = !_flipped;
            } else if (Util::elementIn<TileType, std::size(DANGER_TILES)>(tile->type, DANGER_TILES))
            {
                _vel.x += _flipped ? 0.2 : -0.2;
                _flipped = !_flipped;
            } else {
                vec2<double> checkTilePos {getCenter().x + (_flipped ? -10.0 : 10.0), getCenter().y};
                Tile* tile {world->getTileAt(checkTilePos.x, checkTilePos.y)};
                if (tile != nullptr)
                {
                    _flipped = !_flipped;
                }
                _vel.x += _flipped ? -0.08 : 0.08;
            }
        } else {
            _vel.x *= 0.8;
        }
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

    void loadFromPath(std::string path)
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
                                entity_vec.push_back(new Entity{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, vec2<int>{8, 8}, 0.2, false, entity_name});
                            } else {
                                entity_vec.push_back(new Entity{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, vec2<int>{8, 8}, 0.2, false, "default"});
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
                    entities.push_back(std::vector<Entity*>{new Entity{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, vec2<int>{8, 8}, 0.2, false, entity_name}});
                } else {
                    entities.push_back(std::vector<Entity*>{new Entity{vec2<double>{(double)e["pos"][0], (double)e["pos"][1]}, vec2<double> {0, 0}, vec2<int>{8, 8}, 0.2, false, "default"}});
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