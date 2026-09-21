#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <cassert>
#include "base.h"


u32 compute_shader_create(const char *path) {
    FILE *file = fopen(path, "rb");
    assert(file != NULL);

    char *code;
    size_t file_size, bytes_read;

    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    rewind(file);
    code = (char *)malloc((file_size + 1) * sizeof(char));
    assert(code != NULL);
    
    bytes_read = fread(code, 1, file_size, file);
    code[bytes_read] = '\0';
    
    u32 compute;
    i32 success;
    char info_log[512];

    compute = glCreateShader(GL_COMPUTE_SHADER);
    glShaderSource(compute, 1, (const char**)&code, NULL);
    glCompileShader(compute);
    glGetShaderiv(compute, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(compute, 512, NULL, info_log);
        std::cout << "error::compute::" << info_log << std::endl;
    }

    u32 id = glCreateProgram();
    glAttachShader(id, compute);
    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 512, NULL, info_log);
        std::cout << "error::shader::" << info_log << std::endl;
    }

    glDeleteShader(compute);;
    fclose(file);
    free(code);
    
    return id;
}

u32 shader_create(const char *vertex_path, const char *fragment_path) {
    FILE *vertex_file = fopen(vertex_path, "rb");
    FILE *fragment_file = fopen(fragment_path, "rb");
    assert(vertex_file != NULL);
    assert(fragment_file != NULL);

    char *vertex_code, *fragment_code;
    size_t file_size, bytes_read;

    fseek(vertex_file, 0, SEEK_END);
    file_size = ftell(vertex_file);
    rewind(vertex_file);
    vertex_code = (char *)malloc((file_size + 1) * sizeof(char));
    assert(vertex_code != NULL);
    
    bytes_read = fread(vertex_code, 1, file_size, vertex_file);
    vertex_code[bytes_read] = '\0';

    fseek(fragment_file, 0, SEEK_END);
    file_size = ftell(fragment_file);
    rewind(fragment_file);
    fragment_code = (char *)malloc((file_size + 1) * sizeof(char));
    assert(fragment_code != NULL);
    
    bytes_read = fread(fragment_code, 1, file_size, fragment_file);
    fragment_code[bytes_read] = '\0';
    
    u32 vertex, fragment;
    i32 success;
    char info_log[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, (const char**)&vertex_code, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, info_log);
        std::cout << "error::vertex::" << info_log << std::endl;
    }

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, (const char**)&fragment_code, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, info_log);
        std::cout << "error::fragment::" << info_log << std::endl;
    }

    u32 id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 512, NULL, info_log);
        std::cout << "error::shader::" << info_log << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(fragment);

    free(vertex_code);
    free(fragment_code);

    fclose(vertex_file);
    fclose(fragment_file);
    
    return id;
}

u32 shader_create_with_geometry(const char *vertex_path, const char *geometry_path, const char *fragment_path) {
    FILE *vertex_file = fopen(vertex_path, "rb");
    FILE *geometry_file = fopen(geometry_path, "rb");
    FILE *fragment_file = fopen(fragment_path, "rb");
    assert(vertex_file != NULL);
    assert(fragment_file != NULL);

    char *vertex_code, *geometry_code, *fragment_code;
    size_t file_size, bytes_read;

    fseek(vertex_file, 0, SEEK_END);
    file_size = ftell(vertex_file);
    rewind(vertex_file);
    vertex_code = (char *)malloc((file_size + 1) * sizeof(char));
    assert(vertex_code != NULL);
    bytes_read = fread(vertex_code, 1, file_size, vertex_file);
    vertex_code[bytes_read] = '\0';

    fseek(geometry_file, 0, SEEK_END);
    file_size = ftell(geometry_file);
    rewind(geometry_file);
    geometry_code = (char *)malloc((file_size + 1) * sizeof(char));
    assert(geometry_code != NULL);
    bytes_read = fread(geometry_code, 1, file_size, geometry_file);
    geometry_code[bytes_read] = '\0';

    fseek(fragment_file, 0, SEEK_END);
    file_size = ftell(fragment_file);
    rewind(fragment_file);
    fragment_code = (char *)malloc((file_size + 1) * sizeof(char));
    assert(fragment_code != NULL);
    bytes_read = fread(fragment_code, 1, file_size, fragment_file);
    fragment_code[bytes_read] = '\0';
    
    u32 vertex, geometry, fragment;
    i32 success;
    char info_log[512];

    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, (const char**)&vertex_code, NULL);
    glCompileShader(vertex);
    glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertex, 512, NULL, info_log);
        std::cout << "error::vertex::" << info_log << std::endl;
    }

    geometry = glCreateShader(GL_GEOMETRY_SHADER);
    glShaderSource(geometry, 1, (const char**)&geometry_code, NULL);
    glCompileShader(geometry);
    glGetShaderiv(geometry, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(geometry, 512, NULL, info_log);
        std::cout << "error::geometry::" << info_log << std::endl;
    }
    

    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, (const char**)&fragment_code, NULL);
    glCompileShader(fragment);
    glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragment, 512, NULL, info_log);
        std::cout << "error::fragment::" << info_log << std::endl;
    }

    u32 id = glCreateProgram();
    glAttachShader(id, vertex);
    glAttachShader(id, geometry);
    glAttachShader(id, fragment);
    glLinkProgram(id);
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(id, 512, NULL, info_log);
        std::cout << "error::shader::" << info_log << std::endl;
    }

    glDeleteShader(vertex);
    glDeleteShader(geometry);
    glDeleteShader(fragment);

    free(vertex_code);
    free(geometry_code);
    free(fragment_code);

    fclose(vertex_file);
    fclose(geometry_file);
    fclose(fragment_file);
    
    return id;
}

void shader_bind(u32 shader) {
    glUseProgram(shader);
}

void shader_set_float(u32 shader, const char *name, const float &value) { 
    glUniform1f(glGetUniformLocation(shader, name), value);
}

void shader_set_vec3(u32 shader, const char *name, const glm::vec3 &value) { 
    glUniform3fv(glGetUniformLocation(shader, name), 1, glm::value_ptr(value)); 
}

void shader_set_mat4(u32 shader, const char *name, const glm::mat4 &value) { 
    glUniformMatrix4fv(glGetUniformLocation(shader, name), 1, GL_FALSE, glm::value_ptr(value));
}

#endif