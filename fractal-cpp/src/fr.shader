#version 330 core

out vec4 color;
in vec4 gl_FragCoord;

uniform vec2 screenResolution;
uniform float zoom;
float zoomAdjust = 1 / zoom;
/* The center of the fractal. x, y, ca, cb */
uniform vec4 position; 

const float treashold = 100000000000000000.0f;
const int maxIter = 64;
const float maxIterInverse = 1.0f/maxIter;

vec2 adjust = 2.0f * vec2(1.0f) / screenResolution;


vec2 adjustCoords() {
    return  zoomAdjust * (gl_FragCoord.xy * adjust - 1) - position.xy;
}

vec4 pallete(int iter) {
    float scale = (maxIter - iter) * maxIterInverse;
    return vec4(vec3(1.0f) * scale, 1.0);
}
vec2 pow2(vec2 a) {
    return vec2((a.x * a.x - a.y * a.y + position[2]), (2 * a.x * a.y) + position[3]);
}
void main(){
    vec2 adjust = vec2(1.0f / screenResolution.x, 1.0f / screenResolution.y);
    vec2 coords = adjustCoords();
    for (int i = 0; i < maxIter; i++) {
        coords = pow2(coords);
        if (coords.x > treashold || coords.y > treashold) {
            color = pallete(i);
            return;
        }
    }
    color = vec4(0);
};


