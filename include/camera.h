#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// camera movement directions
enum Camera_Movement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};

// simple camera class for input and view matrix computation
class Camera {
public:
    glm::vec3 position{0.0f, 0.0f, 3.0f};
    glm::vec3 front{0.0f, 0.0f, -1.0f};
    glm::vec3 up{0.0f, 1.0f, 0.0f};
    glm::vec3 right{1.0f, 0.0f, 0.0f};
    glm::vec3 worldUp{0.0f, 1.0f, 0.0f};

    float yaw = -90.0f;    // degrees
    float pitch = 0.0f;    // degrees
    float movementSpeed = 3.0f;
    float mouseSensitivity = 0.1f;
    float zoom = 45.0f;

    // default constructor
    Camera() = default;

    // construct with explicit parameters
    Camera(const glm::vec3& pos, float yawDeg, float pitchDeg, float moveSpeed = 3.0f, float mouseSens = 0.1f, float fov = 45.0f);

    // return lookAt view matrix for current orientation
    glm::mat4 lookAtMatrix() const;

    // update directional vectors from yaw/pitch
    void updateCamera();

    // handle keyboard movement
    void ProcessKeyboard(Camera_Movement direction, float deltaTime);

    // handle mouse movement (offsets in degrees)
    void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true);

    // handle scroll wheel (zoom)
    void ProcessMouseScroll(float yoffset);
};