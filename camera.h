#ifndef CAMERA_H
#define CAMERA_H
#include "myutils3d.hpp"

class Camera
{
public:
    Camera();
    void init();
    void reloadSettings();
    Q orientation() const;
    M4 getMatrix() const;
    M4 getVP() const;
    M4 getProj() const;
    void rotate(float x, float y);
    void move(float f, float r, float u);
    V3 transform(float f, float r, float u) const;
    V3 untransform(const V3 &v) const;

    float fov, aspectRatio, fovMultiplier;
    float zNear, zFar;
    V3 position;
    V3 fw, r;
    float yaw, pitch;
    float sens;
};

#endif // CAMERA_H
