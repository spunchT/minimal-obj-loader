#include "../include/camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <algorithm>

// construct camera with sensible defaults
Camera::Camera(const glm::vec3& pos, float yawDeg, float pitchDeg, float moveSpeed, float mouseSens, float fov)
    : position(pos), yaw(yawDeg), pitch(pitchDeg), movementSpeed(moveSpeed), mouseSensitivity(mouseSens), zoom(fov)
{
    updateCamera();
}

// return lookAt matrix
glm::mat4 Camera::lookAtMatrix() const
{
    return glm::lookAt(position, position + front, up);
}

// update directional vectors from current yaw/pitch
void Camera::updateCamera()
{
    glm::vec3 f;
    f.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    f.y = sin(glm::radians(pitch));
    f.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(f);

    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

// move camera according to direction and delta time
void Camera::ProcessKeyboard(Camera_Movement direction, float deltaTime)
{
    float velocity = movementSpeed * deltaTime;
    if (direction == FORWARD)  position += front * velocity;
    if (direction == BACKWARD) position -= front * velocity;
    if (direction == LEFT)     position -= right * velocity;
    if (direction == RIGHT)    position += right * velocity;
}

// adjust yaw/pitch from mouse movement
void Camera::ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch)
{
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (constrainPitch) {
        pitch = std::max(std::min(pitch, 89.0f), -89.0f);
    }

    updateCamera();
}

// adjust zoom from scroll wheel
void Camera::ProcessMouseScroll(float yoffset)
{
    zoom -= yoffset;
    if (zoom < 1.0f) zoom = 1.0f;
    if (zoom > 45.0f) zoom = 45.0f;
}