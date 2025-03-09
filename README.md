# Defblade 

Defblade (paper-world was the original project name) is a game made from scratch with C++ and SDL2. Inspired by this color palette: [Paper 8 Palette - Lospec Palette List](https://lospec.com/palette-list/paper-8), it was made for the High Seas Hackclub event.

It is precision pixel art platformer with combat based elements, where you parcour across challenging obstacles and kill enemies in order to progress and survive. When you kill enemies you gain money, and when you die you lose money. When you have no money left it's game over. 

![Screenshot of death explosion from Defblade gameplay](https://github.com/snej55/paper-world/blob/main/media/screenshot.png)

More information on the website ([Defblade on itch.io](https://snej55.itch.io/defblade)).

## Play:

[Defblade on itch.io](https://snej55.itch.io/defblade) (You can download it here but this is the official site).

## Linux build instructions
### Try the binaries first!

You should be able to just download the binaries from [itch.io](https://snej55.itch.io/defblade), but in case that doesn't work:

1. Download linux build from itch

2. Make sure sdl2 libraries are installed.

3. Install cmake and ninja-build

4. Make sure headers in include/SDL2 match /usr/include/SDL2

5. Build:

```
# configure cmake
rm -rf bin 
cmake -S . -B bin -G Ninja
# copy assets
cp data -r bin
# build
cd bin 
ninja -j4
# Run!
./Defblade
```

6. Run Defblade!

This should work fine, but please let me know if you have any issues!

### Libraries:

 - [nlohmann JSON library](https://github.com/nlohmann/json)
 - [SDL2 2.30.9](https://libsdl.org/)


