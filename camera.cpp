#include "camera.h"
#include "settings.h"

Camera::Camera()
{
    init();
}

void Camera::init()
{
    reloadSettings();

    position = V3(0.0f, 0.0f, 0.0f);
    fw = V3(0.0f, 1.0f, 0.0f);
    r = V3(1.0f, 0.0f, 0.0f);
    yaw = pitch = 0.0f;
    pitch = 0.0f;

    fovMultiplier = 1.0f;
    aspectRatio = 1.0f;
    zNear = 0.1f;
    zFar = 5000.0f;
    sens = 0.002f;
}

void Camera::reloadSettings()
{
    Settings settings;
    fov = settings.fov / 180.0f * M_PI;
}

Q Camera::orientation() const
{
    V3 up = glm::cross(r, fw);
    Q orientation = glm::angleAxis(0.0f, up);

    Q turn;

    turn = glm::angleAxis(pitch, r);
    orientation = glm::cross(turn, orientation);

    turn = glm::angleAxis(-yaw, up);
    orientation = glm::cross(turn, orientation);

    return orientation;
}

M4 Camera::getMatrix() const
{
    Q orientation = this->orientation();
    V3 center = orientation * fw + position;
    V3 up = orientation * glm::cross(r, fw);
    return glm::lookAt(position, center, up);
}

M4 Camera::getVP() const
{
    M4 proj = glm::perspective(fov * fovMultiplier, aspectRatio, zNear, zFar);
    return proj * getMatrix();
}

M4 Camera::getProj() const
{
    M4 proj = glm::perspective(fov * fovMultiplier, aspectRatio, zNear, zFar);
    return proj;
}

void Camera::rotate(float x, float y)
{
    yaw += x * sens;
    pitch -= y * sens;

    const float maxPitch = 0.499f * 3.1415926535f;

    if (pitch > maxPitch)
        pitch = maxPitch;

    if (pitch < -maxPitch)
        pitch = -maxPitch;
}

void Camera::move(float f, float r, float u)
{
    position += transform(f, r, u);
}

V3 Camera::transform(float f, float r, float u) const
{
    Q orientation = this->orientation();

    V3 vf, vu, vr;
    vf = orientation * this->fw;
    vr = orientation * this->r;
    vu = glm::cross(vr, vf);

    V3 res = V3(0.0f, 0.0f, 0.0f);
    res += vf * f;
    res += vr * r;
    res += vu * u;

    return res;
}

V3 Camera::untransform(const V3 &v) const
{
    Q orientation = glm::conjugate(this->orientation());
    V3 d = orientation * v;
    V3 res = V3(glm::dot(d, this->fw), glm::dot(d, this->r), glm::dot(d, glm::cross(this->r, this->fw)));

    return res;
}
