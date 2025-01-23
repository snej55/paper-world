#ifndef AUDIO_H
#define AUDIO_H

#include "SDL2/SDL_mixer.h"

#include <string>

class Sound
{
private:
    Mix_Chunk* _Sound{nullptr};

public:
    Sound()
    {
    }

    Sound(std::string path)
    {
        loadFromFileWAV(path);
    }

    ~Sound()
    {
        free();
    }

    void free()
    {
        if (_Sound != nullptr)
        {
            Mix_FreeChunk(_Sound);
            _Sound = nullptr;
        }
    }

    void loadFromFileWAV(std::string path)
    {
        free();
        _Sound = Mix_LoadWAV(path.c_str());
    }

    void play(int loops = 0)
    {
        if (_Sound != nullptr)
        {
            Mix_PlayChannel(-1, _Sound, loops);
        }
    }
};

class Music
{
private:
    Mix_Music* _Music {nullptr};

public:
    Music()
    {
    }

    Music(std::string path)
    {
        loadFromFile(path);
    }

    ~Music()
    {
        free();
    }

    void free()
    {
        if (_Music != nullptr)
        {
            Mix_FreeMusic(_Music);
            _Music = nullptr;
        }
    }

    void loadFromFile(std::string path)
    {
        free();
        _Music = Mix_LoadMUS(path.c_str());
    }

    void play()
    {
        if (Mix_PlayingMusic() == 0)
        {
            Mix_PlayMusic(_Music, -1);
        }
    }

    void stop()
    {
        Mix_HaltMusic();
    }
};

#endif