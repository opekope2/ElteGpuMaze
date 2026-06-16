#version 330 core

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

bool hasFlag(uint value, uint flag) {
    return (value & flag) != 0u;
}

void main() {
    vec2 scale = vec2(textureSize(data, 0)) / vec2(res);

    vec2 thisCell = gl_FragCoord.xy;
    vec2 aboveCell = thisCell - vec2(0, 1);
    vec2 leftCell = thisCell - vec2(1, 0);

    ivec2 thisCoords = ivec2(thisCell * scale);
    ivec2 aboveCoords = ivec2(aboveCell * scale);
    ivec2 leftCoords = ivec2(leftCell * scale);

    uint mazeData = texelFetch(data, thisCoords, 0).x;

    bool explored = hasFlag(mazeData, SEARCH_EXPLORED);
    bool frontier = hasFlag(mazeData, SEARCH_FRONTIER);
    bool path = hasFlag(mazeData, SEARCH_PATH);
    bool debug = hasFlag(mazeData, DEBUG);

    bool wallAbove = thisCoords.y != aboveCoords.y && hasFlag(mazeData, WALL_TOP);
    bool wallLeft = thisCoords.x != leftCoords.x && hasFlag(mazeData, WALL_LEFT);
    bool edge = thisCell.x == 0 || thisCell.y == 0 || thisCell.x == res.x - 1 || thisCell.y == res.y - 1;
    bool wall = wallAbove || wallLeft || edge;

    if (wall)
        FragColor = WHITE;
    else if (path)
        FragColor = CYAN;
    else if (frontier)
        FragColor = YELLOW;
    else if (explored)
        FragColor = BLUE;
    else if (debug)
        FragColor = MAGENTA;
    else
        FragColor = BLACK;
}
