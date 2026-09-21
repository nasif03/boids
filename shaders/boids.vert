#version 430 core

const vec3 colors[] = {
    vec3(0.04, 0.10, 0.48), // dark blue
    vec3(0.0,  1.0,  1.0), // cyan
    vec3(0.40, 0.88, 0.40), // light green
    vec3(1.00, 0.30, 0.30),
    vec3(1.00, 0.10, 0.10),
    vec3(0.55, 0.27, 0.07),
    vec3(0.60, 0.20, 0.80),
    vec3(1.00, 0.84, 0.00),
    vec3(1.00, 0.50, 0.00),
    vec3(1.00, 0.40, 0.70),
    vec3(0.40, 1.00, 0.70)
};

struct Boid {
    vec4 pos;
    vec4 vel;
};

layout(std430, binding = 0) buffer boid_buffer {
    Boid boids[];
};

out V_OUT {
    vec3 pos;
    vec3 vel;
    vec3 color;
} v_out;

out vec3 v_color;

void main() {
    v_out.pos = boids[gl_VertexID].pos.xyz;
    v_out.vel = boids[gl_VertexID].vel.xyz;
    v_out.color = colors[gl_VertexID % 1];
}