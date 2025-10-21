#pragma once

#include <string>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// thin shader wrapper for loading, compiling and using opengl shaders
class Shader
{
public:
    unsigned int ID = 0;

    // construct shader from vertex + fragment source file paths
    Shader(const char* vertexPath, const char* fragmentPath);

    // activate the shader program
    void use() const noexcept;

    // set a mat4 uniform by name
    void setMat4(const std::string& name, const glm::mat4& mat) const;

    // set a vec3 uniform by name
    void setVec3(const std::string& name, const glm::vec3& value) const;

private:
    // helper to get uniform location
    int getUniformLocation(const std::string& name) const;
};


