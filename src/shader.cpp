#include "../include/shader.h"
#include <fstream>
#include <sstream>
#include <glad/glad.h>
#include <iostream>

// compile shader helper: compile and return shader id or 0 on failure
static unsigned int CompileShader(unsigned int type, const char* source)
{
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, nullptr);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetShaderInfoLog(shader, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "shader compile error: " << infoLog << "\n";
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

// construct shader program from files
Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    // read vertex and fragment files
    std::ifstream vFile(vertexPath);
    std::ifstream fFile(fragmentPath);
    std::stringstream vStream, fStream;

    if (vFile.is_open()) vStream << vFile.rdbuf();
    if (fFile.is_open()) fStream << fFile.rdbuf();

    std::string vertexCode = vStream.str();
    std::string fragmentCode = fStream.str();

    // remove utf-8 bom if present
    if (vertexCode.size() >= 3 &&
        static_cast<unsigned char>(vertexCode[0]) == 0xEF &&
        static_cast<unsigned char>(vertexCode[1]) == 0xBB &&
        static_cast<unsigned char>(vertexCode[2]) == 0xBF) {
        vertexCode.erase(0, 3);
    }
    if (fragmentCode.size() >= 3 &&
        static_cast<unsigned char>(fragmentCode[0]) == 0xEF &&
        static_cast<unsigned char>(fragmentCode[1]) == 0xBB &&
        static_cast<unsigned char>(fragmentCode[2]) == 0xBF) {
        fragmentCode.erase(0, 3);
    }

    // compile shaders
    unsigned int vert = CompileShader(GL_VERTEX_SHADER, vertexCode.c_str());
    unsigned int frag = CompileShader(GL_FRAGMENT_SHADER, fragmentCode.c_str());

    if (!vert || !frag) {
        // compilation failed, ensure cleanup and keep ID == 0
        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);
        std::cerr << "error: shader compilation failed\n";
        return;
    }

    // link program
    ID = glCreateProgram();
    glAttachShader(ID, vert);
    glAttachShader(ID, frag);
    glLinkProgram(ID);

    int success = 0;
    glGetProgramiv(ID, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[1024];
        glGetProgramInfoLog(ID, sizeof(infoLog), nullptr, infoLog);
        std::cerr << "error: shader link failed: " << infoLog << "\n";
        glDeleteProgram(ID);
        ID = 0;
    }

    // cleanup shader objects
    glDeleteShader(vert);
    glDeleteShader(frag);
}

// activate shader program
void Shader::use() const noexcept
{
    if (ID != 0) glUseProgram(ID);
}

// get uniform location
int Shader::getUniformLocation(const std::string& name) const
{
    return (ID != 0) ? glGetUniformLocation(ID, name.c_str()) : -1;
}

// set mat4 uniform
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const
{
    int loc = getUniformLocation(name);
    if (loc >= 0) glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(mat));
}

// set vec3 uniform
void Shader::setVec3(const std::string& name, const glm::vec3& value) const
{
    int loc = getUniformLocation(name);
    if (loc >= 0) glUniform3fv(loc, 1, glm::value_ptr(value));
}

