// camera.hpp
// camera class

// Copyright 2014 Matthew Chandler

// Permission is hereby granted, free of charge, to any person obtaining a copy of
// this software and associated documentation files (the "Software"), to deal in
// the Software without restriction, including without limitation the rights to
// use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
// the Software, and to permit persons to whom the Software is furnished to do so,
// subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
// COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
// IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
// CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef CAMERA_H
#define CAMERA_H

#include <cmath>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

// free camera - position and direction
class Camera
{
public:
    // Ctor
    // default args put at origin looking down Y axis with Z axis up
    Camera(const glm::vec3 & pos = glm::vec3(0.0f),
        const glm::vec3 & forward = glm::vec3(0.0f, 1.0f, 0.0f),
        const glm::vec3 & up = glm::vec3(0.0f, 0.0f, 1.0f)):
        _pos(pos), _forward(glm::normalize(forward)), _up(glm::normalize(up))
    {
    }

    // set / reset pos / orientation
    // default args put at origin looking down Y axis with Z axis up
    void set(const glm::vec3 & pos = glm::vec3(0.0f),
        const glm::vec3 & forward = glm::vec3(0.0f, 1.0f, 0.0f),
        const glm::vec3 & up = glm::vec3(0.0f, 0.0f, 1.0f))
    {
        _pos = pos;
        _forward = glm::normalize(forward);
        _up = glm::normalize(up);
    }

    // move the position acoording to a vector
    void translate(const glm::vec3 & translation)
    {
        _pos += translation;
    }

    // rotate around an axis
    // angles in radians
    void rotate(const float angle, const glm::vec3 & axis)
    {
        //using Rodrigues' rotation formula
        //http://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
        _forward = glm::normalize(_forward * cosf(angle) + glm::cross(axis, _forward) * sinf(angle)
            + axis * glm::dot(axis, _forward) * (1.0f - cosf(angle)));
        _up = glm::normalize(_up * cosf(angle) + glm::cross(axis, _up) * sinf(angle)
            + axis * glm::dot(axis, _up) * (1.0f - cosf(angle)));
    }

    // get position
    glm::vec3 pos() const
    {
        return _pos;
    }

    // get directional vectors
    glm::vec3 forward() const
    {
        return _forward;
    }
    glm::vec3 up() const
    {
        return _up;
    }
    glm::vec3 right() const
    {
        return glm::cross(_forward, _up);
    }

    // get a view matrix from the camera
    glm::mat4 view_mat() const
    {
        return glm::lookAt(_pos, _pos + _forward, _up);
    }

private:
    // camera pos in world-space
    glm::vec3 _pos;
    // camera orientation vectors
    glm::vec3 _forward, _up;
};

#endif // CAMERA_H
