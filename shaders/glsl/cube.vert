#version 450

layout (location=0) in vec3 pos;
layout (location=1) in vec3 normal;

layout (location=0) out vec3 col;

layout (set=1, binding=0) uniform Matrix {
  mat4 matrix;
};

void main() {
  if (normal.x < 0.0 || normal.y < 0.0 || normal.z < 0.0) {
    col = vec3(1.0) + normal;
  }
  else {
    col = normal;
  }
  gl_Position = matrix * vec4(pos, 1.0);
}
