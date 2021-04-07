#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <math.h>

#include <cmath>

#include "controller.h"
#include "shader_util.h"

int resolutionX;
int resolutionY;

unsigned int mainProgram;
unsigned int computeProgram;

GLuint ssbo;
struct shader_data {
    float min = 1.0;
    float max = 0.0;
    float* data[3840 * 2160]; //biggest supported resolution
} data;

const int SH_EXTRA_FLOATS = 2;

double lastTime;
Controler controller;

void UpdateUniformArguments() {
    glUseProgram(computeProgram);
    int location;

    location = glGetUniformLocation(computeProgram, "maxIter");
    assert(location != -1);
    glUniform1i(location, controller.maxIter);
    
    location = glGetUniformLocation(computeProgram, "center");
    assert(location != -1);
    glUniform4d(location, controller.center[0], controller.center[1], controller.center[2], controller.center[3]);

    location = glGetUniformLocation(computeProgram, "zoom");
    assert(location != -1);
    glUniform1d(location, controller.zoom);

    location = glGetUniformLocation(computeProgram, "rotation");
    assert(location != -1);
    glUniform2f(location, controller.rotation[0], controller.rotation[1]);
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

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    controller.KeyCallback(window, key, scancode, action, mods);
}

static void AttachShaders() {
    mainProgram = glCreateProgram();
    computeProgram = glCreateProgram();

    std::string vertShaderStr = ReadFile("vs.shader");
    std::string fragShaderStr = ReadFile("fr.shader");
    std::string compShaderStr = ReadFile("cs.shader");

    
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
    controller.Update(deltaT);

    updateBuffer();
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

        //glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(mainProgram);

        /* Drawing the whole screen using the shader */
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        /* Poll for and process events */
        glfwPollEvents();
        
        Update();

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

    }

    glfwTerminate();
    return 0;
}
