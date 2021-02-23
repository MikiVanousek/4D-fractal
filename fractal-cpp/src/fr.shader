#version 450 core

out vec4 color;
in vec4 gl_FragCoord;

uniform vec2 screenResolution;
uniform double zoom;
/* The center of the fractal. two visible dimensions - x, y; Two hidden - d, e */
uniform dvec4 center; 
uniform vec2 rotation;

const double treashold = 100000000000000000.0;
const int maxIter = 64;
const float maxIterInverse = 1.0f/maxIter;

dvec2 adjust = 2.0 * vec2(1.0) / screenResolution;
double zoomAdjust = 1.0 / zoom;


dvec4 rotateCenter() {
    double ra = cos(rotation[0]) * center[0] + sin(rotation[0]) * center[2];
    double rb = cos(rotation[1]) * center[1] + sin(rotation[1]) * center[3];
    double rc = sin(rotation[0]) * center[0] + cos(rotation[0]) * center[2];
    double rd = sin(rotation[1]) * center[1] + cos(rotation[1]) * center[3];
    return dvec4(ra, rb, rc, rd);
}

dvec4 rotatedCenter = rotateCenter();

dvec2 adjustCoords() {
    return  zoomAdjust * (gl_FragCoord.xy * adjust - 1);
}


dvec4 rotatedPosition(dvec2 coords) {
    return rotatedCenter + dvec4(coords[0] * cos(rotation[0]), coords[1] * cos(rotation[1]), coords[0] * sin(rotation[0]), coords[1] * sin(rotation[1]));
}

vec4 pallete(int iter) {
    float scale = (maxIter - iter) * maxIterInverse;
    return vec4(vec3(1.0) * scale, 1.0);
}

dvec4 pow2(dvec4 a) {
    return dvec4((a[0] * a[0] - a[1] * a[1] + a[2]), 2 * a[0] * a[1]+ a[3], a[2], a[3]);
}

void main(){
    dvec2 coords = adjustCoords();
    
    dvec4 position = rotatedPosition(coords);
  
    for (int i = 0; i < maxIter; i++) {
        
        position = pow2(position);
        if (position.x > treashold || position.y > treashold) {
            color = pallete(i);
            return;
        }
    }
    color = vec4(0);
};


