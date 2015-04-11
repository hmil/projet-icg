#pragma once
#include "icg_common.h"

class Trackball {
public:
    Trackball() : _radius(1.0f) {}

    // This function is called when the user presses the left mouse button down.
    // x, and y are in [-1, 1]. (-1, -1) is the bottom left corner while (1, 1)
    // is the top right corner.
    void begin_drag(float x, float y) {
      _anchor_pos = vec3(x, y, 0.0f);
      project_onto_surface(_anchor_pos);
    }

    // This fucntion is called while the user moves the curser around while the
    // left mouse button is still pressed.
    // x, and y are in [-1, 1]. (-1, -1) is the bottom left corner while (1, 1)
    // is the top right corner.
    // Returns the rotation of the trackball in matrix form.
    mat4 drag(float x, float y) {
      vec3 current_pos = vec3(x, y, 0.0f);
      project_onto_surface(current_pos);

      float angle = acos(current_pos.dot(_anchor_pos) / (current_pos.norm() * _anchor_pos.norm()));
      mat4 rotation = mat4::Identity();
      vec3 axis = _anchor_pos.cross(current_pos);
      axis.normalize();
      mat3 m;
      m = Eigen::AngleAxisf(angle, axis);

      rotation.block(0,0,3,3) = m;

      return rotation;
    }

private:
    // Projects the point p (whose z coordiante is always empty/zero) onto the
    // trackball surface. If the position at the mouse cursor is outside the
    // trackball, use a hyberbolic sheet as explained in:
    // https://www.opengl.org/wiki/Object_Mouse_Trackball.
    // The trackball radius is given by '_radius'.
    void project_onto_surface(vec3& p) const {
        if (p(0)*p(0) + p(1)*p(1) <= _radius*_radius/2) {
            p(2) = sqrt(_radius * _radius - (p(0)*p(0) + p(1) * p(1)));
        }
        else {
            p(2) = (_radius * _radius / 2) / (sqrt(p(0)*p(0) + p(1)*p(1)));
        }
    }

    float _radius;
    vec3 _anchor_pos;
    mat4 _rotation;
};
