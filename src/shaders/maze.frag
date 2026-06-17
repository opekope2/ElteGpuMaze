#version 330 core

#define HAS_FLAGS(v, f) ((v & f) == f)

layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;
out vec4 FragColor;
uniform usampler2D data;
uniform ivec2 res;

const uint WALL_TOP = 0x01u;
const uint WALL_RIGHT = 0x02u;
const uint WALL_BOTTOM = 0x04u;
const uint WALL_LEFT = 0x08u;

const uint SEARCH_EXPLORED = 0x10u;
const uint SEARCH_FRONTIER = 0x20u;
const uint SEARCH_PATH = 0x40u;

const uint DEBUG = 0x80u;

const vec4 BLACK = vec4(0, 0, 0, 1);
const vec4 RED = vec4(1, 0, 0, 1);
const vec4 GREEN = vec4(0, 1, 0, 1);
const vec4 BLUE = vec4(0, 0, 1, 1);
const vec4 CYAN = vec4(0, 1, 1, 1);
const vec4 MAGENTA = vec4(1, 0, 1, 0);
const vec4 YELLOW = vec4(1, 1, 0, 1);
const vec4 WHITE = vec4(1, 1, 1, 1);

void main() {
    ivec2 adjustedRes = res - ivec2(1);
    ivec2 size = textureSize(data, 0);
    ivec2 minSize = size * ivec2(2);

    if (any(lessThan(adjustedRes, minSize))) {
        FragColor = RED;
        return;
    }

    ivec2 cellSize = adjustedRes / size;
    cellSize = ivec2(min(cellSize.x, cellSize.y));
    ivec2 offset = (adjustedRes - cellSize * size) / 2;
    ivec2 mazeSize = size * cellSize;

    ivec2 cellCoord = ivec2(gl_FragCoord.xy) - offset;

    bool insideNW = all(greaterThanEqual(cellCoord, ivec2(0)));
    bool insideSE = all(lessThan(cellCoord, mazeSize));
    bool edgeSE = insideNW && !insideSE && all(lessThanEqual(cellCoord, mazeSize));

    ivec2 mazeCoord = cellCoord / cellSize;
    uint mazeData = insideNW && insideSE ? texelFetch(data, mazeCoord, 0).x : 0u;

    bvec2 wallCoord = equal(cellCoord % cellSize, ivec2(0));
    bool wall = wallCoord.x && HAS_FLAGS(mazeData, WALL_LEFT) || wallCoord.y && HAS_FLAGS(mazeData, WALL_TOP) || edgeSE;

    if (wall)
        FragColor = WHITE;
    else if (HAS_FLAGS(mazeData, SEARCH_PATH))
        FragColor = CYAN;
    else if (HAS_FLAGS(mazeData, SEARCH_FRONTIER))
        FragColor = YELLOW;
    else if (HAS_FLAGS(mazeData, SEARCH_EXPLORED))
        FragColor = BLUE;
    else if (HAS_FLAGS(mazeData, DEBUG))
        FragColor = MAGENTA;
    else
        FragColor = BLACK;
}
