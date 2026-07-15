#version 450

layout (location=0) in vec3 pos;

layout (set=1, binding=0) uniform Matrix {
  mat4 matrix;
};

const mat4 MVP = mat4(
    vec4( 0.0,      0.0,     -1.001001, -1.0),
    vec4( 0.0,     -2.414214, 0.0,       0.0),
    vec4(-2.414214, 0.0,      0.0,       0.0),
    vec4( 0.0,      0.0,      2.902903,  3.0)
);
void main() {
  gl_Position = matrix * vec4(pos, 1.0);
}
