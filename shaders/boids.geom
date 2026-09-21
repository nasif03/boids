#version 430 core

layout(points) in;
layout(triangle_strip, max_vertices = 4) out;

in V_OUT {
    vec3 pos;
    vec3 vel;
    vec3 color;
} g_in[];

out vec3 f_normal;
out vec3 f_color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    vec3 pos = g_in[0].pos;
    vec3 vel = g_in[0].vel;

    float speed = length(vel);
    vec3 forward = vel / speed;
    
    
    vec3 camera_up = vec3(0.0, 1.0, 0.0);
    vec3 right = normalize(cross(forward, camera_up));
    vec3 up    = cross(right, forward);

    float scale = 0.25;
    float width = 0.1;
    float tail_len = 0.05;

    vec3 v_nose  = pos + forward * scale;
    vec3 v_left  = pos - forward * (scale * 0.2) + right * width;
    vec3 v_right = pos - forward * (scale * 0.2) - right * width;
    vec3 v_tail  = pos - forward * scale;

    mat4 mvp = projection * view * model;

    f_normal = mat3(model) * up;
    f_color  = g_in[0].color;

    gl_Position = mvp * vec4(v_nose, 1.0);   EmitVertex();
    gl_Position = mvp * vec4(v_left, 1.0);   EmitVertex();
    gl_Position = mvp * vec4(v_right, 1.0);  EmitVertex();
    gl_Position = mvp * vec4(v_tail, 1.0);   EmitVertex();

    EndPrimitive();
}