#ifndef POPUPS_H
#define POPUPS_H

#include "./texman.hpp"
#include "./vec2.hpp"

#include <vector>
#include <string>

struct PopUp
{
    vec2<double> pos;
    std::string text;

    double size{255.0}; // size = text alpha
};

class PopUpManager
{
private:
    std::vector<PopUp*> _PopUps{};

public:
    PopUpManager()
    {
    }

    ~PopUpManager()
    {
        free();
    }

    void free()
    {
        for (std::size_t i{0}; i < _PopUps.size(); ++i)
        {
            PopUp* popup{_PopUps[i]};
            if (popup != nullptr)
            {
                delete popup;
                _PopUps[i] = nullptr;
            }
        }
        _PopUps.clear();
    }

    void addPopUp(vec2<double> pos, std::string text)
    {
        _PopUps.push_back(new PopUp{pos, text});
    }

    void update(const double& time_step, SDL_Renderer* renderer, TTF_Font* font)
    {
        for (std::size_t i{0}; i < _PopUps.size(); ++i)
        {
            PopUp* popup{_PopUps[i]};
            if (popup != nullptr)
            {
                popup->pos.y -= time_step;
                popup->size -= time_step;

                if (popup->size >= 1.0)
                {
                    // render popup
                    Texture fontTex{};
                    fontTex.loadFromRenderedText(popup->text.c_str(), SDL_Color{246, 231, 156, static_cast<Uint8>(static_cast<int>(popup->size))}, font, renderer);
                    fontTex.render(static_cast<int>(popup->pos.x), static_cast<int>(popup->pos.y), renderer);
                } else {
                    // cleanup
                    delete popup;
                    _PopUps[i] = nullptr;
                }
            }
        }

        _PopUps.erase(std::remove_if(_PopUps.begin(), _PopUps.end(), [](PopUp* popup){return popup == nullptr;}), _PopUps.end());
    }
};

#endif