#version 330 core

out vec4 color;
in vec4 gl_FragCoord;

uniform vec2 screenResolution;
uniform float zoom;
/* The center of the fractal. two visible dimensions - x, y; Two hidden - d, e */
uniform vec4 center; 
uniform vec2 rotation;

const float treashold = 100000000000000000.0f;
const int maxIter = 64;
const float maxIterInverse = 1.0f/maxIter;

vec2 adjust = 2.0f * vec2(1.0f) / screenResolution;
float zoomAdjust = 1 / zoom;

vec2 screenCoords() {
    return gl_FragCoord.xy * adjust - 1;
}

vec2 adjustCoords() {
    rotation;
    // todo tri center
    return  zoomAdjust * screenCoords();
}

vec4 rotatedPosition(vec2 coords) {
    // todo vyndat sin, aby se zbytecne nepocital

    return vec4((coords[0] + center[0]) * cos(rotation[0]), (coords[1] + center[1]) * cos(rotation[1]), (coords[0] + center[0]) * sin(rotation[0]) + center[2], (coords[1] + center[1]) * sin(rotation[1]) + center[3]);
}

vec4 pallete(int iter) {
    float scale = (maxIter - iter) * maxIterInverse;
    return vec4(vec3(1.0f) * scale, 1.0);
}

vec4 pow2(vec4 a) {
    return vec4((a[0] * a[0] - a[1] * a[1] + a[2]), 2 * a[0] * a[1]+ a[3], a[2], a[3]);
}
void main(){
    vec2 coords = adjustCoords();
    
    vec4 position = rotatedPosition(coords);
  
    for (int i = 0; i < maxIter; i++) {
        
        position = pow2(vec4(position));
        if (position.x > treashold || position.y > treashold) {
            color = pallete(i);
            return;
        }
    }
    color = vec4(0);
};


