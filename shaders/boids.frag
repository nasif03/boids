#version 430 core

in vec3 f_color;
in vec3 f_normal;

out vec4 frag_color;

void main() {
    frag_color = vec4(f_color, 1.0);
}