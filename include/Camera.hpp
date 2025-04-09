#ifndef CAMERA_HPP
#define CAMERA_HPP

#include <glm/glm.hpp>

class Camera
{
public:
    Camera(glm::vec3 eye, glm::vec3 viewDirection, glm::vec3 upVector);
    Camera();
    glm::mat4 getViewMatrix();
    void moveForward(float speed);
    void moveBackward(float speed);
    void moveLeft(float speed);
    void moveRight(float speed);

private:
    glm::vec3 eye;
    glm::vec3 viewDirection;
    glm::vec3 upVector;
};

#endif