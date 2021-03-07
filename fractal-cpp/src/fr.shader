#version 450 core

/* Takes in the position of the pixel as parametr. */
in vec4 gl_FragCoord;
/* Outputs color... */
out vec4 color;

const double treashold = 1000.0 * 1000.0 * 1000.0 ;
/* The number of iterations performed for each pixel, before considering it bounded. */
const int maxIter = 64;

layout(std430, binding = 1) buffer bufferIn
{
    float data[];
};

const vec4 notEscapedColor = vec4(1.0, 0.0, 0.0, 0.0);

/* Width and height of the window in pixels. */
uniform vec2 screenResolution;
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
    return  zoomAdjust * (gl_FragCoord.xy * adjust - 1);
}


dvec4 rotatedPosition(dvec2 coords) {
    ra = rotationCos[0] * coords[0];
    rb = rotationCos[1] * coords[1];
    rc = rotationSin[0] * coords[0];
    rd = rotationSin[1] * coords[1];

    return rotatedCenter + dvec4(ra, rb, rc, rd);
}

vec4 pallete(int iter, double distanceSQ) {
    float log_zn = log(float(distanceSQ)) / 2.0;
    float nu = log(log_zn / log(2.0)) / log2; 
    float iterAdj = 1.0 - nu + float(iter);
    float scale = iterAdj / (maxIter);
    return vec4(vec3(1.0) * scale, 1.0);
}

dvec4 pow2(dvec4 a ) {
    return dvec4((a[0] * a[0] - a[1] * a[1] + a[2]), 2.0 * a[0] * a[1]+ a[3], a[2], a[3]);
}

double lengthSQ(dvec4 pos) {
    return pos.x * pos.x + pos.y * pos.y;
}

void main(){
    color = vec4(0.0, data[int(gl_FragCoord.x)], 0.0, 1.0);
    return;
    dvec2 coords = adjustCoords();

    dvec4 position = rotatedPosition(coords);
  
    for (i = 0; i < maxIter; i++) {
        position = pow2(position);
        double length = lengthSQ(position);
        if (length > treashold) {
            color = pallete(i, lengthSQ(position));
            return;
        }
    }
    color = notEscapedColor;
};