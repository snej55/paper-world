#ifndef WINDOW_H
#define WINDOW_H

#include <SDL2/SDL.h>
#include "./constants.hpp"

class Window
{
private:
    SDL_Window* _Window;
    int _width;
    int _height;
    
    bool _mouse_focus;
    bool _keyboard_focus;
    bool _Fullscreen;
    bool _Minimized;

public:
    Window()
    {
        _Window = nullptr;
        _mouse_focus = false;
        _keyboard_focus = false;
        _Fullscreen = false;
        _Minimized = false;
        _width = 0;
        _height = 0;
    }

    bool init()
    {
        _Window = SDL_CreateWindow("Paper World", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, WINDOW_WIDTH, WINDOW_HEIGHT, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
        if (_Window != NULL)
        {
            _mouse_focus = true;
            _keyboard_focus = true;
            _width = WINDOW_WIDTH;
            _height = WINDOW_HEIGHT;
        }
        return _Window != NULL;
    }

    SDL_Renderer* createRenderer()
    {
        return SDL_CreateRenderer(_Window, -1, SDL_RENDERER_ACCELERATED, SDL_RENDERER_PRESENTVSYNC);
    }

    void handleEvent(SDL_Event& e)
    {
        if (e.type == SDL_WINDOWEVENT)
        {
            bool updateCaption {false};
            switch (e.window.event)
            {
                case SDL_WINDOWEVENT_SIZE_CHANGED:
                    _width = e.window.data1;
                    _height = e.window.data2;
                    break;
                case SDL_WINDOWEVENT_ENTER:
                    _mouse_focus = true;
                    updateCaption = true;
                case SDL_WINDOWEVENT_LEAVE:
                
            }
        }
    }
};

#endif