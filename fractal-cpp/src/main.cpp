#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <string>
#include <iostream>
#include <fstream>

#include <cassert>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <linmath.h>
#include <math.h>

#include <cmath>

int resolutionX = 1000;
int resolutionY = 500;

unsigned int mainProgram;
unsigned int computeProgram;

GLuint ssbo;
struct shader_data {
    float min = 1.0;
    float max = 0.0;
    float* data[3840 * 2160]; //biggest supported resolution
} data;

const int SH_EXTRA_FLOATS = 2;

/* The center of the fractal. x, y, ca, cb */
double center[4] = { 0.0, 0.0, 0.0, 0.0 };
float currentMovementSpeed[4] = { 0.0f, 0.0f , 0.0f, 0.0f };

double zoom = 1.0;
float currentZoomSpeed = 0.0f;

float rotation[2] = { 3.14 /2, 3.14 / 2 } ;
//float rotation[2] = { 0.0, 0.0};
float currentRotationSpeed[2] = { 0.0f, 0.0f };

const float MOVEMENT_SPEED[4] = { 0.5f, 0.5f, 0.1f, 0.1f };
const float ZOOM_SPEED = 1.5f;
const float ROTATION_SPEED = 0.5f;

double lastTime;

void UpdateUniformArguments() {
    glUseProgram(computeProgram);
    int location;
    
    location = glGetUniformLocation(computeProgram, "center");
    assert(location != -1);
    glUniform4d(location, center[0], center[1], center[2], center[3]);

    location = glGetUniformLocation(computeProgram, "zoom");
    assert(location != -1);
    glUniform1d(location, zoom);

    location = glGetUniformLocation(computeProgram, "rotation");
    assert(location != -1);
    glUniform2f(location, rotation[0], rotation[1]);
    glUseProgram(mainProgram);
}

void SetupUniformArguments() {
    glUseProgram(computeProgram);
    int location = glGetUniformLocation(computeProgram, "screenResolution");
    assert(location != -1);
    glUniform2i(location, resolutionX, resolutionY);

    glUseProgram(mainProgram);
    location = glGetUniformLocation(mainProgram, "screenResolution");
    assert(location != -1);
    glUniform2i(location, resolutionX, resolutionY);

    UpdateUniformArguments();
}

static void ErrorCallback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static float getDir(int action, bool positiveDir) {
    float dir = 1.0f;
    if (!positiveDir)
        dir *= -1.0f;
    if (action == GLFW_RELEASE)
        dir *= -1.0f;
    return dir;
}

static void UpdateMovementSpeed(int index, int action, bool positiveDir) {
    currentMovementSpeed[index] += getDir(action, positiveDir) * MOVEMENT_SPEED[index];

    printf("Movement speed updated: %.1f\t%.1f\t%.1f\t%.1f\n", currentMovementSpeed[0], currentMovementSpeed[1], currentMovementSpeed[2], currentMovementSpeed[3]);
}

static void UpdateRotationSpeed(int index, int action, bool positiveDir) {
    currentRotationSpeed[index] += getDir(action, positiveDir) * ROTATION_SPEED;

    printf("Rotation speed updated: %.1f\t%.1f\n", currentRotationSpeed[0], currentRotationSpeed[1]);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
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
        case GLFW_KEY_Y:
            UpdateRotationSpeed(0, action, true);
            break;
        case GLFW_KEY_H:
            UpdateRotationSpeed(0, action, false);
            break;
        case GLFW_KEY_U:
            UpdateRotationSpeed(1, action, true);
            break;
        case GLFW_KEY_J:
            UpdateRotationSpeed(1, action, false);
            break;

        case GLFW_KEY_LEFT_SHIFT:
            currentZoomSpeed += ZOOM_SPEED * getDir(action, true);
            break;
        case GLFW_KEY_LEFT_CONTROL:
            currentZoomSpeed += ZOOM_SPEED * getDir(action, false);
            break;
        }
       
        printf("Key pressed: %d\n", key);
        printf("Zoom: %d\n", zoom);
    }
}

std::string ReadFile(const char* filePath) {
    std::string content;
    std::ifstream fileStream(filePath, std::ios::in);

    if (!fileStream.is_open()) {
        std::cerr << "Could not read file " << filePath << ". File does not exist." << std::endl;
        return "";
    }

    std::string line = "";
    while (!fileStream.eof()) {
        std::getline(fileStream, line);
        content.append(line + "\n");
    }

    fileStream.close();
    return content;
}

static unsigned int CompileShader(const std::string& source, unsigned int type) {
    char* shaderTypeName;
    if (type == GL_VERTEX_SHADER)
        shaderTypeName = (char*)"Vertex Shader";
    else if(type == GL_FRAGMENT_SHADER)
        shaderTypeName = (char*)"Fragment Shader";
    else
        shaderTypeName = (char*)"Compute Shader";
    

    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)_malloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);

        printf("%s attachment failed!\n", shaderTypeName);
        std::cout << message << std::endl;

        glDeleteShader(id);

    }
    else {
        printf("%s succsessfully attached!\n", shaderTypeName);
    }
    return id;
}

static void AttachShaders() {
    mainProgram = glCreateProgram();
    computeProgram = glCreateProgram();

    std::string vertShaderStr = ReadFile("src/vs.shader");
    std::string fragShaderStr = ReadFile("src/fr.shader");
    std::string compShaderStr = ReadFile("src/cs.shader");

    
    unsigned int vs = CompileShader(vertShaderStr, GL_VERTEX_SHADER);
    unsigned int fs = CompileShader(fragShaderStr, GL_FRAGMENT_SHADER);
    unsigned int cs = CompileShader(compShaderStr, GL_COMPUTE_SHADER);
 
    glAttachShader(mainProgram, vs);
    glAttachShader(mainProgram, fs);
    glAttachShader(computeProgram, cs);
    
    glLinkProgram(computeProgram);
    glValidateProgram(computeProgram);

    glLinkProgram(mainProgram);
    glValidateProgram(mainProgram);

    glUseProgram(computeProgram);
    
    /* Removes shaders from cash. */
    glDeleteShader(vs);
    glDeleteShader(cs);
    glDeleteShader(fs);

}

void printInfo() {
    /* Getting version info */
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);
    int work_grp_cnt[3];

    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &work_grp_cnt[0]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &work_grp_cnt[1]);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &work_grp_cnt[2]);

    printf("max global (total) work group counts x:%i y:%i z:%i\n",
        work_grp_cnt[0], work_grp_cnt[1], work_grp_cnt[2]);
}

void setupBuffer() {
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glNamedBufferStorage(ssbo, sizeof(float) * (resolutionX * resolutionY + SH_EXTRA_FLOATS), &data, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_DYNAMIC_STORAGE_BIT); //sizeof(data) only works for statically sized C/C++ arrays.
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo);
}

void updateBuffer() {
    float d[] = { data.min, data.max };
    glNamedBufferSubData(ssbo, 0, 2 * sizeof(float), &d);
}

static void Update() {
    if (lastTime == NULL)
        lastTime = glfwGetTime();
    double deltaT = glfwGetTime() - lastTime;
    lastTime = glfwGetTime();

    for (int i = 0; i < 4; i++) {
        center[i] += deltaT * currentMovementSpeed[i] / zoom;
    }

    for (int i = 0; i < 2; i++) {
        rotation[i] += deltaT * currentRotationSpeed[i] / zoom;
    }

    zoom *= pow(2, deltaT * currentZoomSpeed);

    UpdateUniformArguments();
}

int main(void)
{
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit()) {
        return -1;
    }
   
    /* Create a windowed mode window and its OpenGL context */
 
    const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    resolutionX = mode->width;
    resolutionY = mode->height;
    window = glfwCreateWindow(resolutionX, resolutionY, "Fractal Renderer", glfwGetPrimaryMonitor(), NULL);

    /* Seting up callbacks */
    glfwSetErrorCallback(ErrorCallback);
    glfwSetKeyCallback(window, KeyCallback);

  
    if (!window)
    {
        fprintf(stderr, "ERROR: Could not open the window!");
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);

    /* Initialze Glew */
    if (glewInit() != GLEW_OK) {
        return -1;
    }

    printInfo();

    glfwSwapInterval(1);

    setupBuffer();

    AttachShaders();
 
    SetupUniformArguments();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glUseProgram(computeProgram);
        glDispatchCompute(resolutionX, resolutionY, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(mainProgram);

        /* Drawing the whole screen using the shader */
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        /* Poll for and process events */
        glfwPollEvents();
        
        updateBuffer();
        Update();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

    }

    glfwTerminate();
    return 0;
}
