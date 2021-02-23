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

int RESOLUTION_X = 1000;
int RESOLUTION_Y = 500;

unsigned int program;

/* The center of the fractal. x, y, ca, cb */
float position[4] = { 0.0f, 0.0f, 0.5f, 0.0f };
float positionSpeed[4] = { 0.0f, 0.0f , 0.0f, 0.0f };

const float movementSpeed = 0.1f;

double lastTime;

void UpdateUniformArguments() {
    int location;

    location = glGetUniformLocation(program, "position");
    assert(location != -1);
    glUniform4f(location, position[0], position[1], position[2], position[3]);
}

void SetupUniformArguments() {
    int location = glGetUniformLocation(program, "screenResolution");
    assert(location != -1);
    glUniform2f(location, (float)RESOLUTION_X, (float)RESOLUTION_Y);

    UpdateUniformArguments();
}

static void ErrorCallback(int error, const char* description) {
    fprintf(stderr, "Error: %s\n", description);
}

static void UpdateSpeed(int index, int action, bool positiveDir) {
    float dir = 1.0f;
    if (!positiveDir)
        dir *= -1.0f;
    if (action == GLFW_RELEASE)
        dir *= -1.0f;
    positionSpeed[index] += dir * movementSpeed;

    printf("Speed updated: %.1f\t%.1f\t%.1f\t%.1f\n", positionSpeed[0], positionSpeed[1], positionSpeed[2], positionSpeed[3]);
}

static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    if (action == GLFW_PRESS || action == GLFW_RELEASE) {
        switch (key) {
        case GLFW_KEY_Q:
            UpdateSpeed(0, action, true);
            break;
        case GLFW_KEY_A:
            UpdateSpeed(0, action, false);
            break;
        case GLFW_KEY_W:
            UpdateSpeed(1, action, true);
            break;
        case GLFW_KEY_S:
            UpdateSpeed(1, action, false);
            break;
        case GLFW_KEY_E:
            UpdateSpeed(2, action, true);
            break;
        case GLFW_KEY_D:
            UpdateSpeed(2, action, false);
            break;
        case GLFW_KEY_R:
            UpdateSpeed(3, action, true);
            break;
        case GLFW_KEY_F:
            UpdateSpeed(3, action, false);
            break;
        }
        printf("Key pressed: %d\n", key);
    }
}

static void Update() {
    if (lastTime == NULL)
        lastTime = glfwGetTime();
    double deltaT = glfwGetTime() - lastTime;
    lastTime = glfwGetTime();

    for (int i = 0; i < 4; i++) {
        position[i] += positionSpeed[i] * deltaT;
    }

    UpdateUniformArguments();
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
    else
        shaderTypeName = (char*)"Fragment Shader";
    

    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);
    
    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)alloca(length * sizeof(char));
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
    program = glCreateProgram();

    std::string vertShaderStr = ReadFile("src/vs.shader");
    std::string fragShaderStr = ReadFile("src/fr.shader");

    
    unsigned int vs = CompileShader(vertShaderStr, GL_VERTEX_SHADER);
    unsigned int fs = CompileShader(fragShaderStr, GL_FRAGMENT_SHADER);
 
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    
    glLinkProgram(program);
    glValidateProgram(program);
    
    /* Removes shaders from cash. */
    glDeleteShader(vs);
    glDeleteShader(fs);

    glUseProgram(program);
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
    RESOLUTION_X = mode->width;
    RESOLUTION_Y = mode->height;
    window = glfwCreateWindow(RESOLUTION_X, RESOLUTION_Y, "Fractal Renderer", glfwGetPrimaryMonitor(), NULL);

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

    /* Getting version info */
    const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
    const GLubyte* version = glGetString(GL_VERSION); // version as a string
    printf("Renderer: %s\n", renderer);
    printf("OpenGL version supported %s\n", version);


    glfwSwapInterval(1);

    AttachShaders();
    SetupUniformArguments();

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        glClear(GL_COLOR_BUFFER_BIT);

        //glDrawArrays(GL_TRIANGLES, 0, 3);

        /* Drawing the whole screen using the shader */
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

        /* Swap front and back buffers */
        glfwSwapBuffers(window);

        /* Poll for and process events */
        glfwPollEvents();

        Update();
    }

    glfwTerminate();
    return 0;
}
