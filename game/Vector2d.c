#include "Vector2d.h"
#include "../global.h"

#include <math.h>

Vector2d Vector2d_zero = {0.0f, 0.0f};

Vector2d Vector2d_multiply(Vector2d v, float scalar) {
  Vector2d ret;
  ret.x = v.x * scalar;
  ret.y = v.y * scalar;
  return ret;
}

Vector2d Vector2d_add(Vector2d v1, Vector2d v2) {
  Vector2d ret;
  ret.x = v1.x + v2.x;
  ret.y = v1.y + v2.y;
  return ret;
}

Vector2d Vector2d_subtract(Vector2d v1, Vector2d v2) {
  Vector2d ret;
  ret.x = v1.x - v2.x;
  ret.y = v1.y - v2.y;
  return ret;
}

Vector2d Vector2d_normalize(Vector2d v) {
  float mag = Vector2d_magnitude(v);
  if (mag == 0.0f) {
    return Vector2d_zero;
  }
  return Vector2d_multiply(v, 1.0f / mag);
}

float Vector2d_dot(Vector2d v1, Vector2d v2) {
  return v1.x * v2.x + v1.y * v2.y;
}

float Vector2d_magnitude_squared(Vector2d v) { return v.x * v.x + v.y * v.y; }

float Vector2d_magnitude(Vector2d v) {
  return sqrt(Vector2d_magnitude_squared(v));
}

// This looks weird but it depends on the fact that atan can only return
// angles between -PI/2 and PI/2
// and we have to avoid an exception when dividing by zero
float Vector2d_angle(Vector2d v) {
  if (v.x == 0.0f) {
    if (v.y > 0.0f) {
      return M_PI / 2.0f;
    } else {
      return 3.0f * M_PI / 2.0f;
    }
  }
  float tan = v.y / v.x;
  float angle = atan(tan);
  if (v.x < 0.0f) {
    angle += M_PI;
  } else if (v.y < 0.0f) {
    angle += 2 * M_PI;
  }
  return angle;
}
