#pragma once
#include <cassert>

#include <GL/glew.h>
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

#include <stdint.h>
#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>

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
    else if (type == GL_FRAGMENT_SHADER)
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
