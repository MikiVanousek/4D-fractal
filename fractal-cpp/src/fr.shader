#version 330 core

out vec4 color;
in vec4 gl_FragCoord;

uniform vec2 screenResolution;
float ca = 0.5;
float cb = 0.3;

const float treashold = 1000000000;
const int iter = 10;

vec2 adjust = 2.0f * vec2(1.0f) / screenResolution;


vec2 adjustCoords() {
    return gl_FragCoord.xy * adjust - 1;
}

vec2 pow2(vec2 a) {
    return vec2((a.x * a.x - a.y * a.y + ca), (2 * a.x * a.y) + cb);
}
void main(){
    vec2 adjust = vec2(1.0f / screenResolution.x, 1.0f / screenResolution.y);
    vec2 coords = adjustCoords();
    for (int i = 0; i < iter; i++) {
        coords = pow2(coords);
        if (coords.x > treashold || coords.y > treashold) {
            color = vec4(1);
            return;
        }
    }
    color = vec4(0);
};


