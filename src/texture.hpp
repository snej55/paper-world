#ifndef TEXTURE_H
#define TEXTURE_H

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_video.h"

#include "./constants.hpp"

#include <iostream>
#include <string>
#include <cassert>

#ifndef SCALE_FACTOR_DEF
#define SCALE_FACTOR_DEF

constexpr int SCALE_FACTOR{3};
#endif

class Texture
{
private:
    SDL_Texture* mTexture;

    SDL_Surface* mSurfacePixels;

    void* mRawPixels;
    int mRawPitch;

    int mWidth;
    int mHeight;

public:
    Texture()
    {
        // initialize
        mTexture = NULL;
        mSurfacePixels = NULL;
        mRawPixels = NULL;
        mRawPitch = 0;
        mWidth = 0;
        mHeight = 0;
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

        return mTexture != NULL;
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
            mSurfacePixels = SDL_ConvertSurfaceFormat( loadedSurface, SDL_GetWindowPixelFormat( window ), 0 );
            if( mSurfacePixels == NULL )
            {
                printf( "Unable to convert loaded surface to display format! SDL Error: %s\n", SDL_GetError() );
            }
            else
            {
                mWidth = mSurfacePixels->w;
                mHeight = mSurfacePixels->h;
            }

            SDL_FreeSurface( loadedSurface );
        }

        return mSurfacePixels != NULL;
    }

    bool loadFromPixels(SDL_Renderer* renderer)
    {
        if (mSurfacePixels == NULL)
        {
            std::cout << "TEXTURE::LOADFROMPIXELS No pixels loaded!\n";
        } else {
            SDL_SetColorKey(mSurfacePixels, SDL_TRUE, SDL_MapRGB(mSurfacePixels->format, 0, 0, 0));
            mTexture = SDL_CreateTextureFromSurface(renderer, mSurfacePixels);
            assert(mTexture != NULL && SDL_GetError());
            mWidth = mSurfacePixels->w;
            mHeight = mSurfacePixels->h;
            SDL_FreeSurface(mSurfacePixels);
            mSurfacePixels = NULL;
        }
        return mTexture != NULL;
    }

    bool createBlank(int width, int height, SDL_Renderer* renderer, SDL_TextureAccess access)
    {
        free();

        mTexture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, access, width, height);
        if (mTexture == NULL)
        {
            std::cout << "Unable to create texture!\n";
        } else {
            mWidth = width;
            mHeight = height;
        }

        return mTexture != NULL;
    }

    void free()
    {
        if (mTexture != NULL)
        {
            SDL_DestroyTexture(mTexture);
            mTexture = NULL;
            mWidth = 0;
            mHeight = 0;
        }

        if (mSurfacePixels != NULL)
        {
            SDL_FreeSurface(mSurfacePixels);
            mSurfacePixels = NULL;
        }
    }

    void setColor(Uint8 red, Uint8 green, Uint8 blue)
    {
        SDL_SetTextureColorMod(mTexture, red, green, blue);
    }

    void setBlendMode(SDL_BlendMode blending)
    {
        SDL_SetTextureBlendMode(mTexture, blending);
    }

    void setAlpha(Uint8 alpha)
    {
        SDL_SetTextureAlphaMod(mTexture, alpha);
    }

    void render(int x, int y, SDL_Renderer* renderer, SDL_Rect* clip = NULL)
    {
        SDL_Rect renderQuad {x, y, mWidth, mHeight};

        if (clip != NULL)
        {
            renderQuad.w = clip->w;
            renderQuad.h = clip->h;
        }

        renderQuad.w *= SCALE_FACTOR;
        renderQuad.h *= SCALE_FACTOR;

        SDL_RenderCopy(renderer, mTexture, clip, &renderQuad);
    }

    void render(int x, int y, SDL_Renderer* renderer, double angle, SDL_Point* center, SDL_RendererFlip flip, SDL_Rect* clip = NULL, int scale_factor = NULL)
    {
        SDL_Rect renderQuad {x, y, mWidth, mHeight};

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

        SDL_RenderCopyEx(renderer, mTexture, clip, &renderQuad, angle, center, flip);
    }

    void renderClean(int x, int y, SDL_Renderer* renderer, int scale_factor)
    {
        SDL_Rect renderQuad {x, y, mWidth, mHeight};
        renderQuad.w *= scale_factor;
        renderQuad.h *= scale_factor;
        SDL_RenderCopy(renderer, mTexture, NULL, &renderQuad);
    }

    void renderClean(int x, int y, SDL_Renderer* renderer)
    {
        SDL_Rect renderQuad {x, y, mWidth, mHeight};
        SDL_RenderCopy(renderer, mTexture, NULL, &renderQuad);
    }

    void setAsRenderTarget(SDL_Renderer* renderer)
    {
        SDL_SetRenderTarget(renderer, mTexture);
    }

    int getWidth()
    {
        return mWidth;
    }

    int getHeight()
    {
        return mHeight;
    }

    Uint32* getPixels32()
    {
        Uint32* pixels {NULL};
        if (mSurfacePixels != NULL)
        {

            pixels = static_cast<Uint32*>(mSurfacePixels->pixels);
        }
        return pixels;
    }

    Uint32 getPitch32()
    {
        Uint32 pitch {0};
        if (mSurfacePixels != NULL)
        {
            // divide by 4 because 32bits/4bytes per pixel
            pitch = mSurfacePixels->pitch / 4;
        }
        return pitch;
    }

    Uint32 mapRGBA(Uint8 r, Uint8 g, Uint8 b, Uint8 a)
    {
        Uint32 pixel{0};

        if (mSurfacePixels != NULL)
        {
            pixel = SDL_MapRGBA(mSurfacePixels->format, r, g, b, a);
        }
        return pixel;
    }

    void copyRawPixels32(void* pixels)
    {
        if (mRawPixels != NULL)
        {
            memcpy(mRawPixels, pixels, mRawPitch * mHeight);
        }
    }

    bool lockTexture()
    {
        bool success {true};
        if (mRawPixels != NULL)
        {
            std::cout << "Texture is already locked!\n";
            success = false;
        } else {
            if (SDL_LockTexture(mTexture, NULL, &mRawPixels, &mRawPitch) != 0)
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
        if (mRawPixels == NULL)
        {
            std::cout << "Texture is not locked!\n";
            success = false;
        } else {
            SDL_UnlockTexture(mTexture);
            mRawPixels = NULL;
            mRawPitch = 0;
        }
        return success;
    }
};

#endif
