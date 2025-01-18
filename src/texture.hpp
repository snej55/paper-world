#ifndef TEXTURE_H
#define TEXTURE_H

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_video.h"
#include "SDL2/SDL_ttf.h"

#include "./constants.hpp"

#include <iostream>
#include <string>
#include <cassert>

#ifndef SCALE_FACTOR_DEF
#define SCALE_FACTOR_DEF

constexpr int SCALE_FACTOR{1};
#endif

class Texture
{
private:
    SDL_Texture* _Texture;

    SDL_Surface* _SurfacePixels;

    void* _RawPixels;
    int _RawPitch;

    int _Width;
    int _Height;

public:
    Texture()
    {
        // initialize
        _Texture = NULL;
        _SurfacePixels = NULL;
        _RawPixels = NULL;
        _RawPitch = 0;
        _Width = 0;
        _Height = 0;
    }

    ~Texture()
    {
        // Deallocate
        free();
    }

    bool loadFromFile(std::string path, SDL_Window* window, SDL_Renderer* renderer)
    {
        if (!loadPixelsFromFile(path, window))
        {
            std::cout << "Failed to load pixels for '" << path.c_str() << "'!\n";
        } else {
            if (!loadFromPixels(renderer))
            {
                std::cout << "Failed to load textures from pixels from '" << path.c_str() << "'!\n";
            }
        }

        return _Texture != NULL;
    }

    bool loadPixelsFromFile(std::string path, SDL_Window* window)
    {
        free();

        SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
        if( loadedSurface == NULL )
        {
            printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
        }
        else
        {
            _SurfacePixels = SDL_ConvertSurfaceFormat( loadedSurface, SDL_GetWindowPixelFormat( window ), 0 );
            if( _SurfacePixels == NULL )
            {
                printf( "Unable to convert loaded surface to display format! SDL Error: %s\n", SDL_GetError() );
            }
            else
            {
                _Width = _SurfacePixels->w;
                _Height = _SurfacePixels->h;
            }

            SDL_FreeSurface( loadedSurface );
        }

        return _SurfacePixels != NULL;
    }

    bool loadFromPixels(SDL_Renderer* renderer)
    {
        if (_SurfacePixels == NULL)
        {
            std::cout << "TEXTURE::LOADFROMPIXELS No pixels loaded!\n";
        } else {
            SDL_SetColorKey(_SurfacePixels, SDL_TRUE, SDL_MapRGB(_SurfacePixels->format, 0, 0, 0));
            _Texture = SDL_CreateTextureFromSurface(renderer, _SurfacePixels);
            assert(_Texture != NULL && SDL_GetError());
            _Width = _SurfacePixels->w;
            _Height = _SurfacePixels->h;
            SDL_FreeSurface(_SurfacePixels);
            _SurfacePixels = NULL;
        }
        return _Texture != NULL;
    }

    bool loadFromRenderedText(const char* text, SDL_Color textColor, TTF_Font* font, SDL_Renderer* renderer)
    {
        free();
        SDL_Surface* textSurface = TTF_RenderText_Solid(font, text, textColor);
        if (textSurface == NULL)
        {
            std::cout << "Unable to render text surface! SDL_ttf error: " << TTF_GetError << '\n';
        } else {
            _Texture = SDL_CreateTextureFromSurface(renderer, textSurface);
            if (_Texture == NULL)
            {
                std::cout << "Unable to create texture form rendered text! SDL_Error: " << SDL_GetError() << '\n';
            } else {
                _Width = textSurface->w;
                _Height = textSurface->h;
            }

            SDL_FreeSurface(textSurface);
        }

        return _Texture != NULL;
    }

    bool createBlank(int width, int height, SDL_Renderer* renderer, SDL_TextureAccess access)
    {
        free();

        _Texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, access, width, height);
        if (_Texture == NULL)
        {
            std::cout << "Unable to create texture!\n";
        } else {
            _Width = width;
            _Height = height;
        }

        return _Texture != NULL;
    }

    void free()
    {
        if (_Texture != NULL)
        {
            SDL_DestroyTexture(_Texture);
            _Texture = NULL;
            _Width = 0;
            _Height = 0;
        }

        if (_SurfacePixels != NULL)
        {
            SDL_FreeSurface(_SurfacePixels);
            _SurfacePixels = NULL;
        }
    }

    void setColor(Uint8 red, Uint8 green, Uint8 blue)
    {
        SDL_SetTextureColorMod(_Texture, red, green, blue);
    }

    void setBlendMode(SDL_BlendMode blending)
    {
        SDL_SetTextureBlendMode(_Texture, blending);
    }

    void setAlpha(Uint8 alpha)
    {
        SDL_SetTextureAlphaMod(_Texture, alpha);
    }

    void render(int x, int y, SDL_Renderer* renderer, SDL_Rect* clip = NULL)
    {
        SDL_Rect renderQuad {x, y, _Width, _Height};

        if (clip != NULL)
        {
            renderQuad.w = clip->w;
            renderQuad.h = clip->h;
        }

        renderQuad.w *= SCALE_FACTOR;
        renderQuad.h *= SCALE_FACTOR;

        SDL_RenderCopy(renderer, _Texture, clip, &renderQuad);
    }

    void render(int x, int y, SDL_Renderer* renderer, double angle, SDL_Point* center, SDL_RendererFlip flip, SDL_Rect* clip = NULL, int scale_factor = 0)
    {
        SDL_Rect renderQuad {x, y, _Width, _Height};

        if (clip != NULL)
        {
            renderQuad.w = clip->w;
            renderQuad.h = clip->h;
        }

        renderQuad.w *= SCALE_FACTOR;
        renderQuad.h *= SCALE_FACTOR;

        if (scale_factor)
        {
            renderQuad.w *= scale_factor;
            renderQuad.h *= scale_factor;
        }

        SDL_RenderCopyEx(renderer, _Texture, clip, &renderQuad, angle, center, flip);
    }

    void renderClean(int x, int y, SDL_Renderer* renderer, int scale_factor)
    {
        SDL_Rect renderQuad {x, y, _Width, _Height};
        renderQuad.w *= scale_factor;
        renderQuad.h *= scale_factor;
        SDL_RenderCopy(renderer, _Texture, NULL, &renderQuad);
    }

    void renderClean(int x, int y, SDL_Renderer* renderer)
    {
        SDL_Rect renderQuad {x, y, _Width, _Height};
        SDL_RenderCopy(renderer, _Texture, NULL, &renderQuad);
    }

    void setAsRenderTarget(SDL_Renderer* renderer)
    {
        SDL_SetRenderTarget(renderer, _Texture);
    }

    int getWidth()
    {
        return _Width;
    }

    int getHeight()
    {
        return _Height;
    }

    Uint32* getPixels32()
    {
        Uint32* pixels {NULL};
        if (_SurfacePixels != NULL)
        {

            pixels = static_cast<Uint32*>(_SurfacePixels->pixels);
        }
        return pixels;
    }

    Uint32 getPitch32()
    {
        Uint32 pitch {0};
        if (_SurfacePixels != NULL)
        {
            // divide by 4 because 32bits/4bytes per pixel
            pitch = _SurfacePixels->pitch / 4;
        }
        return pitch;
    }

    Uint32 mapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
    {
        Uint32 pixel{0};

        if (_SurfacePixels != NULL)
        {
            pixel = SDL_MapRGBA(_SurfacePixels->format, r, g, b, a);
        }
        return pixel;
    }

    void copyRawPixels32(void* pixels)
    {
        if (_RawPixels != NULL)
        {
            memcpy(_RawPixels, pixels, _RawPitch * _Height);
        }
    }

    bool lockTexture()
    {
        bool success {true};
        if (_RawPixels != NULL)
        {
            std::cout << "Texture is already locked!\n";
            success = false;
        } else {
            if (SDL_LockTexture(_Texture, NULL, &_RawPixels, &_RawPitch) != 0)
            {
                std::cout << "Unable to lock texture! SDL_Error: " << SDL_GetError() << '\n';
                success = false;
            }
        }
        return success;
    }

    bool unlockTexture()
    {
        bool success {true};
        if (_RawPixels == NULL)
        {
            std::cout << "Texture is not locked!\n";
            success = false;
        } else {
            SDL_UnlockTexture(_Texture);
            _RawPixels = NULL;
            _RawPitch = 0;
        }
        return success;
    }

    SDL_Texture* getTexture()
    {
        return _Texture;
    }
};

#endif
