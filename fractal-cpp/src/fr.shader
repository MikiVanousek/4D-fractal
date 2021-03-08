#version 450 core

in vec4 gl_FragCoord;
out vec4 color;
uniform vec2 minMax;

layout(std430, binding = 1) buffer bufferIn
{
    float min;
    float max;
    float data[];
};

float adjust = 1.0 / (max - min);

const vec4 notEscapedColor = vec4(1.0, 0.0, 0.0, 0.0);


void main(){
    int index = int(gl_FragCoord.x) + 1920 * int(gl_FragCoord.y);

    if (data[index] == -1) {
        color = notEscapedColor;
        return;
    }

    float value = (data[index] - min) / (max - min);

    

    color = vec4(1.0) * value;
};