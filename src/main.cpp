#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <assert.h>
#include <random>

// constants

const int SCR_WIDTH = 1920;
const int SCR_HEIGHT = 1080;

const int BOIDS = 400;
const float VISIBLE_RANGE = 20.0f;
const float PROTECTED_RANGE = 7.0f;
const float CENTERING_FACTOR = 0.0002f;
const float AVOID_FACTOR = 0.02f;
const float MATCHING_FACTOR = 0.02f;
const float TURN_FACTOR = 0.2f;
const float BIAS = 0.001f;
const float MAX_BIAS = 0.01f;
const float BIAS_INCREMENT = 0.00004f;
const float MIN_SPEED = 12.0f;
const float MAX_SPEED = 6.0f;
const float MARGIN = 50.0f;

// timing

float delta_time = 0.0f;
float last_frame = 0.0f;

// camera

glm::vec3 camera_pos = glm::vec3(-30.0f, 0.0f, 0.0f);
glm::vec3 camera_front = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 camera_up = glm::vec3(0.0f, 1.0f, 0.0f);
float yaw = 0.0f, pitch = 0.0f;
float last_x = SCR_WIDTH / 2.0f, last_y = SCR_HEIGHT / 2.0f;
bool first_mouse = true;

// functions

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_input(GLFWwindow *window);
unsigned int shader_init(const char *vertex_path, const char *fragment_path);

int main() {
    // init glfw
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // create window and context
    
    GLFWwindow *window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "boids", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);
    
    // glad : load all function pointers

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }

    // random number generator
    
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(-20, 20);

    // boid vertex data

    float vertices[] = {
        0.0f, 0.0f, 0.4f,
        0.0f, 0.0f, -0.4f,
        0.7f, 0.1f, 0.0f,
        -0.7f, 0.1f, 0.0f,
        0.0f, -0.1f, 0.4f,
        0.0f, -0.1f, -0.4f
    };

    unsigned int indices[] = {
        0, 1, 2,
        0, 1, 3,
        4, 5, 3,
        4, 5, 2,
        1, 5, 3,
        1, 5, 2,
        0, 4, 3,
        0, 4, 2
    };

    // boid positions

    glm::vec3 boid_pos[BOIDS], boid_vel[BOIDS];
    for (int i = 0; i < BOIDS; i++) {
        boid_pos[i] = glm::vec3(
            (float)dist(mt),
            (float)dist(mt),
            (float)dist(mt)
        );
        boid_vel[i] = glm::normalize(glm::vec3(
            (float)dist(mt),
            (float)dist(mt),
            (float)dist(mt)
        ));
    }

    // shaders

    unsigned int shader_id = shader_init("../shaders/v1.vs", "../shaders/f1.fs");
    glUseProgram(shader_id);

    // matrices

    glm::mat4 view;
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 projection;
    projection = glm::perspective(glm::radians(90.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    int model_loc = glGetUniformLocation(shader_id, "model");
    int view_loc = glGetUniformLocation(shader_id, "view");
    int projection_loc = glGetUniformLocation(shader_id, "projection");
    glUniformMatrix4fv(projection_loc, 1, GL_FALSE, glm::value_ptr(projection));

    // buffer objects

    unsigned int vao, vbo, ebo;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    glEnableVertexAttribArray(0);
    glBindVertexArray(0);

    // render loop

    glEnable(GL_DEPTH_TEST);

    while (!glfwWindowShouldClose(window)) {
        // timing
        
        float current_frame = (float)glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        // input

        process_input(window);

        // update boid positions

        for (int i = 0; i < BOIDS; i++) {
            glm::vec3 close = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 avg_vel = glm::vec3(0.0f, 0.0f, 0.0f);
            glm::vec3 avg_pos = glm::vec3(0.0f, 0.0f, 0.0f);
            int neighbors = 0;

            for (int j = 0; j < BOIDS; j++) {
                if (i == j) continue;
                if (glm::distance(boid_pos[i], boid_pos[j]) < PROTECTED_RANGE) {
                    close += boid_pos[i] - boid_pos[j];
                }
                if (glm::distance(boid_pos[i], boid_pos[j]) < VISIBLE_RANGE) {
                    avg_vel += boid_vel[j];
                    avg_pos += boid_pos[j];
                    neighbors++;
                }
            }
            boid_vel[i] += close * AVOID_FACTOR;
            if (neighbors > 0) {
                avg_vel /= neighbors;
                avg_pos /= neighbors;

                boid_vel[i] += (avg_vel - boid_vel[i]) * MATCHING_FACTOR;
                boid_vel[i] += (avg_pos - boid_pos[i]) * CENTERING_FACTOR;
            }
            float speed = glm::length(boid_vel[i]);
            if (speed > MAX_SPEED) {
                boid_vel[i] *= MAX_SPEED / speed;
            }
            if (speed < MIN_SPEED) {
                boid_vel[i] *= MIN_SPEED / speed;
            }

            if (boid_pos[i].x < -MARGIN)
                boid_vel[i].x += TURN_FACTOR;
            if (boid_pos[i].x > MARGIN)
                boid_vel[i].x -= TURN_FACTOR;
            if (boid_pos[i].y < -MARGIN)
                boid_vel[i].y += TURN_FACTOR;
            if (boid_pos[i].y > MARGIN)
                boid_vel[i].y -= TURN_FACTOR;
            if (boid_pos[i].z < -MARGIN)
                boid_vel[i].z += TURN_FACTOR;
            if (boid_pos[i].z > MARGIN)
                boid_vel[i].z -= TURN_FACTOR;

            boid_pos[i] += boid_vel[i] * delta_time;
        }
        
        // render

        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glUseProgram(shader_id);
        glBindVertexArray(vao);

        view = glm::lookAt(camera_pos, camera_pos + camera_front, camera_up);
        glUniformMatrix4fv(view_loc, 1, GL_FALSE, glm::value_ptr(view));

        // draw boids
        
        for (int i = 0; i < BOIDS; i++) {
            model = glm::mat4(1.0f);
            glm::mat4 rotation = glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f),
                                             boid_vel[i],
                                             camera_up);

            model = glm::translate(model, boid_pos[i]) * rotation;

            glUniformMatrix4fv(model_loc, 1, GL_FALSE, glm::value_ptr(model));
            glDrawElements(GL_TRIANGLES, 24, GL_UNSIGNED_INT, 0);   
        }

        
        // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        // buffer swap and poll IO events

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}


void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos) {
    if (first_mouse) {
        last_x = xpos;
        last_y = ypos;
        first_mouse = false;
    }

    float xoffset = xpos - last_x;
    float yoffset = last_y - ypos;
    last_x = xpos, last_y = ypos;

    float sensitivity = 0.1f;
    xoffset *= sensitivity;
    yoffset *= sensitivity;
    
    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f) pitch = 89.0f;
    if (pitch < -89.0f) pitch = -89.0f;

    glm::vec3 dir;
    dir.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    dir.y = sin(glm::radians(pitch));
    dir.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    camera_front = glm::normalize(dir);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {

}

void process_input(GLFWwindow *window) {
    float camera_speed = 10.0f * delta_time;

    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
    
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_pos += camera_speed * camera_front;
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_pos -= camera_speed * camera_front;
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_pos -= glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_pos += glm::normalize(glm::cross(camera_front, camera_up)) * camera_speed;
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera_pos.y += camera_speed;
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
        camera_pos.y -= camera_speed;
}

unsigned int shader_init(const char *vertex_path, const char *fragment_path) {
    FILE *vertex_file = fopen(vertex_path, "rb");
    assert(vertex_file != NULL);
    FILE *fragment_file = fopen(fragment_path, "rb");
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
    
    unsigned int vertex, fragment;
    int success;
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

    unsigned int id = glCreateProgram();
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

    fclose(vertex_file);
    fclose(fragment_file);

    return id;
}