#ifndef CONSTANTS_H
#define CONSTANTS_H

inline constexpr int START_COINS{1000};

inline constexpr int TILE_SIZE {8};
inline constexpr int CHUNK_SIZE {9};

inline constexpr int MAX_ENTITIES {512}; // max number of entities per entity manager

// level dimensions in chunks
inline constexpr int LEVEL_WIDTH {14};
inline constexpr int LEVEL_HEIGHT {7};

inline constexpr int WINDOW_WIDTH {660};
inline constexpr int WINDOW_HEIGHT {660};
inline constexpr int SCR_WIDTH {220};
inline constexpr int SCR_HEIGHT {220};

#define SCALE_FACTOR_DEF
inline constexpr int SCALE_FACTOR {1};

#endif
