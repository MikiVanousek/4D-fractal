#version 450 core

in vec4 gl_FragCoord;
out vec4 color;
uniform ivec2 screenResolution;

layout(std430, binding = 1) buffer bufferIn
{
    float min;
    float max;
    float data[];
};

float adjust = 1.0 / (max - min);

const vec4 notEscapedColor = vec4(0.0);

const int NUMBER_OF_COLORS = 4;
const float PART = 1.0 / (NUMBER_OF_COLORS - 1);
const float CRI = 1 / 255.0;
const vec3 COLORS[NUMBER_OF_COLORS] = {
    //vec3(1.0, 1.0, 1.0),
    vec3(1.0, 1.0, 0.0),
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 0.0, 1.0),
    vec3(0.0, 1.0, 0.0),
    
};

vec4 lerp(vec3 c1, vec3 c2, float progress) {
    return vec4(c1 + (c2 - c1) * progress, 1.0);
}

void main(){
    int index = int(gl_FragCoord.x) + screenResolution.x * int(gl_FragCoord.y);
   
    if (data[index] == -1) {
        color = notEscapedColor;
        return;
    }

    float value = (data[index] - min) / (max - min);
    if (value < 0) value = 0;
    if (value > 1) value = 1;

    for (int i = 0; i < NUMBER_OF_COLORS - 1; i++) {
        if (value < PART) {
            color = lerp(COLORS[i], COLORS[i + 1], value / PART);
            return;
        }
        value -= PART;
    }
    

    color = vec4(1.0) * value;
};