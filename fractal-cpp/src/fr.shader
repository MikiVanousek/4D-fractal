#version 330 core

out vec4 color;
in vec4 gl_FragCoord;

uniform vec2 screenResolution;
vec2 adjust = 2.0f * vec2(1.0f) / screenResolution;


vec2 adjustCoords() {
    return gl_FragCoord.xy * adjust - 1;
}

void main(){
    vec2 adjust = vec2(1.0f / screenResolution.x, 1.0f / screenResolution.y);
    vec2 coord = adjustCoords();
    color = vec4(coord.xy, 0.0, 1.0);
};
