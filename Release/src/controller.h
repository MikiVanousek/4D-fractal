#define _USE_MATH_DEFINES
#include <cstdio>
#include <cmath>

class Controler {
public:
    int maxIter = 128;
	/* The center of the fractal. x, y, ca, cb */
	double center[4] = { 0.0, 0.0, 0.0, 0.0 };
	float currentMovementSpeed[4] = { 0.0f, 0.0f , 0.0f, 0.0f };

	double zoom = 1.0;
	float currentZoomSpeed = 0.0f;

    /* Rotation of the axis: x ->ca, y -> cb in pi.*/
	float rotation[2] = { 0.0, 0.0};
    float currentRotationLeft[2] = { 0.0f, 0.0f };

	const float MOVEMENT_SPEED[4] = { 0.5f, 0.5f, 0.1f, 0.1f };
	const float ZOOM_SPEED = 1.5f;
	const float ROTATION_SPEED = 0.5f;

    static float getDir(int action, bool positiveDir) {
        float dir = 1.0f;
        if (!positiveDir)
            dir *= -1.0f;
        if (action == GLFW_RELEASE)
            dir *= -1.0f;
        return dir;
    }

    void UpdateMovementSpeed(int index, int action, bool positiveDir) {
        currentMovementSpeed[index] += getDir(action, positiveDir) * MOVEMENT_SPEED[index];
        //printf("Movement speed updated: %.1f\t%.1f\t%.1f\t%.1f\n", currentMovementSpeed[0], currentMovementSpeed[1], currentMovementSpeed[2], currentMovementSpeed[3]);
    }

    void UpdateRotationLeft(int index, int action, bool positiveDir) {
        currentRotationLeft[index] += getDir(action, positiveDir) * 0.5;
        //printf("Rotation left updated: %.1f\t%.1f\n", currentRotationLeft[0], currentRotationLeft[1]);
    }


    void Update(double deltaT) {
        for (int i = 0; i < 4; i++) {
            center[i] += deltaT * currentMovementSpeed[i] / zoom;
        }

        for (int i = 0; i < 2; i++) {
            if (currentRotationLeft[i] != 0) {
                float rotationChange = ROTATION_SPEED * deltaT;

                if (currentRotationLeft[i] > 0) {
                    if (currentRotationLeft[i] - rotationChange <= 0) {
                        rotation[i] += currentRotationLeft[i];
                        currentRotationLeft[i] = 0;
                    }
                    else {
                        rotation[i] += rotationChange;
                        currentRotationLeft[i] -= rotationChange;
                    }
                }

                if (currentRotationLeft[i] < 0) {
                    if (currentRotationLeft[i] + rotationChange >= 0) {
                        rotation[i] += currentRotationLeft[i];
                        currentRotationLeft[i] = 0;
                    }
                    else {
                        rotation[i] -= rotationChange;
                        currentRotationLeft[i] += rotationChange;
                    }
                }
            }
            
        }

        zoom *= pow(2, deltaT * currentZoomSpeed);

        printf("\nMax Iterations: %7.0d | Position (x, y, ca, cb): %.4f, %.4f, %.4f, %.4f | Zoom %6.0f | Rotation ([x,ca], [y,cb]): %.4fPI, %.4fPI\n", maxIter, center[0], center[1], center[2], center[3], zoom, rotation[0], rotation[1]);
    }

    void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
        if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        if (action == GLFW_PRESS || action == GLFW_RELEASE) {
            switch (key) {
                case GLFW_KEY_D:
                    UpdateMovementSpeed(0, action, true);
                    break;
                case GLFW_KEY_A:
                    UpdateMovementSpeed(0, action, false);
                    break;
                case GLFW_KEY_W:
                    UpdateMovementSpeed(1, action, true);
                    break;
                case GLFW_KEY_S:
                    UpdateMovementSpeed(1, action, false);
                    break;
                case GLFW_KEY_R:
                    UpdateMovementSpeed(2, action, true);
                    break;
                case GLFW_KEY_F:
                    UpdateMovementSpeed(2, action, false);
                    break;
                case GLFW_KEY_T:
                    UpdateMovementSpeed(3, action, true);
                    break;
                case GLFW_KEY_G:
                    UpdateMovementSpeed(3, action, false);
                    break;
                case GLFW_KEY_LEFT_SHIFT:
                    currentZoomSpeed += ZOOM_SPEED * getDir(action, true);
                    break;
                case GLFW_KEY_LEFT_CONTROL:
                    currentZoomSpeed += ZOOM_SPEED * getDir(action, false);
                    break;
            }
            if (action == GLFW_PRESS) {
                switch(key){
                    case GLFW_KEY_RIGHT_SHIFT:
                        maxIter *= 2;
                        break;
                    case GLFW_KEY_RIGHT_CONTROL:
                        if (maxIter > 1)
                            maxIter /= 2;
                        break;
                    case GLFW_KEY_Y:
                        UpdateRotationLeft(0, action, true);
                        break;
                    case GLFW_KEY_H:
                        UpdateRotationLeft(0, action, false);
                        break;
                    case GLFW_KEY_U:
                        UpdateRotationLeft(1, action, true);
                        break;
                    case GLFW_KEY_J:
                        UpdateRotationLeft(1, action, false);
                        break;
                    case GLFW_KEY_ENTER:
                        for (int i = 0; i < 4; i++)
                            center[i] = 0;
                        zoom = 1;
                        break;
                }
            }
        }
    }

};
