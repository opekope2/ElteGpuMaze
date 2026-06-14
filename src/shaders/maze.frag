#version 330 core

layout(origin_upper_left, pixel_center_integer) in vec4 gl_FragCoord;
out vec4 FragColor;
uniform usampler2D data;
uniform ivec2 res;

const uint WALL_TOP = 0x01u;
const uint WALL_RIGHT = 0x02u;
const uint WALL_BOTTOM = 0x04u;
const uint WALL_LEFT = 0x08u;
const uint DEBUG = 0x80u;

const vec4 BLACK = vec4(0, 0, 0, 1);
const vec4 WHITE = vec4(1, 1, 1, 1);
const vec4 GREEN = vec4(0, 1, 0, 1);

bool isWall(uint walls, uint wall) {
    return (walls & wall) != 0u;
}

void main() {
    vec2 scale = vec2(textureSize(data, 0)) / vec2(res);

    vec2 thisCell = gl_FragCoord.xy;
    vec2 aboveCell = thisCell - vec2(0, 1);
    vec2 leftCell = thisCell - vec2(1, 0);

    ivec2 thisCoords = ivec2(thisCell * scale);
    ivec2 aboveCoords = ivec2(aboveCell * scale);
    ivec2 leftCoords = ivec2(leftCell * scale);

    uint walls = texelFetch(data, thisCoords, 0).x;

    bool wallAbove = thisCoords.y != aboveCoords.y && isWall(walls, WALL_TOP);
    bool wallLeft = thisCoords.x != leftCoords.x && isWall(walls, WALL_LEFT);
    bool debug = isWall(walls, DEBUG);
    bool edge = thisCell.x == 0 || thisCell.y == 0 || thisCell.x == res.x - 1 || thisCell.y == res.y - 1;
    bool wall = wallAbove || wallLeft || edge;

    FragColor = wall ? WHITE : (debug ? GREEN : BLACK);
}
