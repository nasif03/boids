#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <iostream>
#include <random>
#include <cassert>
#include "base.h"
#include "camera.h"
#include "shader.h"

// functions

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void process_input(GLFWwindow *window);
unsigned int shader_init(const char *vertex_path, const char *fragment_path);

// global

camera player_cam;

// structs

struct Boid {
    glm::vec4 pos;
    glm::vec4 vel;
};

int main() {
    #pragma region initialize OpenGL
    
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // glad : load all function pointers
    
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return -1;
    }
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    #pragma endregion
    
    // random number generator
    
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_real_distribution<float> dist(-1, 1);

    // initialize boid positions and velocity

    Boid boids[NUM_BOIDS];
    for (u32 i = 0; i < NUM_BOIDS; i++) {
        boids[i].pos = { dist(mt) * 30.0f, dist(mt) * 10.0f, dist(mt) * 30.0f, 0.0f };
        boids[i].vel = { dist(mt), dist(mt), dist(mt), 0.0f };
    }
    
    // store in ssbo
    
    u32 vao, ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, NUM_BOIDS * sizeof(Boid), boids, GL_DYNAMIC_COPY);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo);

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);


    u32 render_shader = shader_create_with_geometry("shaders/boids.vert", "shaders/boids.geom", "shaders/boids.frag");
    u32 compute_shader = compute_shader_create("shaders/boids.comp");
    
    // render loop

    while (!glfwWindowShouldClose(window)) {
        // timing
        
        float current_frame = (float)glfwGetTime();
        delta_time = current_frame - last_frame;
        last_frame = current_frame;

        // input

        process_input(window);

        // update boid positions

        shader_bind(compute_shader);
        shader_set_float(compute_shader, "u_delta_time", delta_time);

        u32 num_groups = (NUM_BOIDS + 255) / 256;
        glDispatchCompute(num_groups, 1, 1);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT);
        
        // render

        glClearColor(0.7f, 0.7f, 0.75f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        shader_bind(render_shader);
        glm::mat4 model = glm::mat4(1.0);
        glm::mat4 view = camera_view_mat(player_cam);
        glm::mat4 projection = glm::perspective(glm::radians(90.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
        shader_set_mat4(render_shader, "model", model);
        shader_set_mat4(render_shader, "view", view);
        shader_set_mat4(render_shader, "projection", projection);

        glBindVertexArray(vao);
        glDrawArrays(GL_POINTS, 0, NUM_BOIDS);

        // draw boids

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
    float x = (float)xpos, y = (float)ypos;
    if (first_mouse) {
        last_x = x;
        last_y = y;
        first_mouse = false;
    }
    float xoffset = x - last_x;
    float yoffset = last_y - y;
    last_x = x;
    last_y = y;
    camera_process_mouse(player_cam, xoffset, yoffset);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset) {

}

void process_input(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera_process_keyboard(player_cam, CAM_FORWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera_process_keyboard(player_cam, CAM_BACKWARD, delta_time);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera_process_keyboard(player_cam, CAM_LEFT, delta_time);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera_process_keyboard(player_cam, CAM_RIGHT, delta_time);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera_process_keyboard(player_cam, CAM_UP, delta_time);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT))
        camera_process_keyboard(player_cam, CAM_DOWN, delta_time);
}
