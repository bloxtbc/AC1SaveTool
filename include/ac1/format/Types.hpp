#pragma once

#include <cstdint>

namespace ac1 {

struct Vector2 {
    float x;
    float y;
};

struct Vector3 {
    float x;
    float y;
    float z;
};

struct Vector4 {
    float x;
    float y;
    float z;
    float w;
};

struct Matrix3x3 {
    Vector3 m01;
    Vector3 m02;
    Vector3 m03;
};

struct Matrix4x4 {
    Vector4 m01;
    Vector4 m02;
    Vector4 m03;
    Vector4 m04;
};

} // namespace ac1
