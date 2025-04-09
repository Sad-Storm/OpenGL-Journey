#include "Camera.hpp"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <iostream>

Camera::Camera(glm::vec3 eye, glm::vec3 viewDirection, glm::vec3 upVector)
{
    this->eye = eye;
    this->viewDirection = viewDirection;
    this->upVector = upVector;
}

Camera::Camera()
{
    eye = glm::vec3(0.0f, 0.0f, 5.0f);
    viewDirection = glm::vec3(0.0f, 0.0f, -1.0f);
    upVector = glm::vec3(0.0f, 1.0f, 0.0f);
}

glm::mat4 Camera::getViewMatrix()
{
    return glm::lookAt(eye, eye + viewDirection, upVector);
}

void Camera::mouseLook(int deltaX, int deltaY)
{
    const float sensitivity = 0.1f;

    // Calculate yaw rotation matrix
    glm::mat4 yawRotation = glm::rotate(glm::mat4(1.0f), glm::radians(-deltaX * sensitivity), upVector);

    // Apply rotations to viewDirection
    viewDirection = glm::vec3(yawRotation * glm::vec4(viewDirection, 0.0f));

    viewDirection = glm::normalize(viewDirection);
}

void Camera::moveForward(float speed)
{
    eye.z -= speed;
}
void Camera::moveBackward(float speed)
{
    eye.z += speed;
}
void Camera::moveLeft(float speed)
{
    eye.x -= speed;
}
void Camera::moveRight(float speed)
{
    eye.x += speed;
}