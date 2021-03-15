#version 450
layout(local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(std430, binding = 1) buffer destBuffer
{
	float min;
	float max;
	float data[];
} outBuffer;
/* Outputs color... */

const double treashold = 1000.0 * 1000.0;
/* The number of iterations performed for each pixel, before considering it bounded. */
const int maxIter = 256;

layout(std430, binding = 1) buffer bufferIn
{
    float min;
    float max;
    float data[];
};

const vec4 notEscapedColor = vec4(1.0, 0.0, 0.0, 0.0);

/* Width and height of the window in pixels. */
uniform ivec2 screenResolution;
/* The zoom factor by which everything is scaled. */
uniform double zoom;

/* The center of the fractal. two visible dimensions - x, y; Two hidden - d, e */
uniform dvec4 center;
uniform vec2 rotation;

/* Optimization: variables are calcualted just once, not for every pixel */
const float maxIterInverse = 1.0f / maxIter;
double zoomAdjust = 1.0 / zoom;
dvec2 adjust = 2.0 * dvec2(1.0) / screenResolution;
float log2 = log(2.0);

float rotationSin[2] = { sin(rotation[0]), sin(rotation[1]) };
float rotationCos[2] = { cos(rotation[0]), cos(rotation[1]) };

/* Optimization: variables are allocated just once, not for every pixel */
dvec2 coords;
dvec4 position;
int i;
double ra, rb, rc, rd;
float scale;



/* Converts the from screen-oriented coordinates to absolute coordinates */
dvec4 rotateCenter() {
    ra = rotationCos[0] * center[0] + rotationSin[0] * center[2];
    rb = rotationCos[1] * center[1] + rotationSin[1] * center[3];
    rc = rotationSin[0] * center[0] + rotationCos[0] * center[2];
    rd = rotationSin[1] * center[1] + rotationCos[1] * center[3];
    return dvec4(ra, rb, rc, rd);
}
dvec4 rotatedCenter = rotateCenter();

dvec2 adjustCoords() {
    return  zoomAdjust * (gl_WorkGroupID.xy * adjust - 1);
}


dvec4 rotatedPosition(dvec2 coords) {
    ra = rotationCos[0] * coords[0];
    rb = rotationCos[1] * coords[1];
    rc = rotationSin[0] * coords[0];
    rd = rotationSin[1] * coords[1];

    return rotatedCenter + dvec4(ra, rb, rc, rd);
}

dvec4 pow2(dvec4 a) {
    return dvec4((a[0] * a[0] - a[1] * a[1] + a[2]), 2.0 * a[0] * a[1] + a[3], a[2], a[3]);
}

double lengthSQ(dvec4 pos) {
    return pos.x * pos.x + pos.y * pos.y;
}

void main() {
    screenResolution;
    int index = int(gl_WorkGroupID.x + screenResolution.x * gl_WorkGroupID.y);
    dvec2 coords = adjustCoords();

    dvec4 position = rotatedPosition(coords);

    for (i = 0; i < maxIter; i++) {
        position = pow2(position);
        double length = lengthSQ(position);
        if (length > treashold) {
            float log_zn = log(float(length)) / 2.0;
            float nu = log(log_zn / log(2.0)) / log2;
            float iterAdj = 1.0 - nu + float(i);
            float scale = iterAdj / float(maxIter);
            if (scale < 0)
                data[index] = -2;
            data[index] = scale;
            if (scale > max) max = scale;
            if (scale < min && scale > 0) min = scale;
            return;
        }
    }
    data[index] = -1;
};