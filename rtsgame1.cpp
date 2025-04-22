#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define GRID_SIZE 10
#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

// Shader sources
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec2 aPos;\n"
    "void main() { gl_Position = vec4(aPos, 0.0, 1.0); }\n";

const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec4 color;\n"
    "void main() { FragColor = color; }\n";

// Unit structure
typedef struct {
    float x, y;  // Position in grid units
    bool selected;
} Unit;

Unit units[] = {{2.0f, 2.0f, false}, {5.0f, 5.0f, false}};
int unitCount = 2;

void checkShaderCompile(GLuint shader) {
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, NULL, infoLog);
        printf("Shader compilation failed: %s\n", infoLog);
    }
}

void checkProgramLink(GLuint program) {
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(program, 512, NULL, infoLog);
        printf("Program linking failed: %s\n", infoLog);
    }
}

// Mouse callback for selecting/moving units
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        float x = (xpos / WINDOW_WIDTH) * 2.0f - 1.0f;
        float y = 1.0f - (ypos / WINDOW_HEIGHT) * 2.0f;

        // Convert screen coords to grid coords
        float gridX = (x + 1.0f) * GRID_SIZE / 2.0f;
        float gridY = (y + 1.0f) * GRID_SIZE / 2.0f;

        bool anySelected = false;
        for (int i = 0; i < unitCount; i++) {
            float dx = units[i].x - gridX;
            float dy = units[i].y - gridY;
            if (dx * dx + dy * dy < 0.5f) {  // Rough hitbox
                units[i].selected = true;
                anySelected = true;
            } else if (!mods) {  // Deselect if no modifier (e.g., Shift)
                units[i].selected = false;
            }
        }

        if (!anySelected) {
            for (int i = 0; i < unitCount; i++) {
                if (units[i].selected) {
                    units[i].x = gridX;
                    units[i].y = gridY;
                    units[i].selected = false;
                }
            }
        }
    }
}

int main() {
    // Initialize GLFW
    if (!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    GLFWwindow *window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "RTS Game", NULL, NULL);
    if (!window) {
        printf("Failed to create GLFW window\n");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // Initialize GLEW
    if (glewInit() != GLEW_OK) {
        printf("Failed to initialize GLEW\n");
        return -1;
    }

    // Compile shaders
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkShaderCompile(vertexShader);

    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkShaderCompile(fragmentShader);

    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    checkProgramLink(shaderProgram);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    // Vertex data for a quad
    float quadVertices[] = {
        -0.05f, -0.05f,
         0.05f, -0.05f,
         0.05f,  0.05f,
        -0.05f,  0.05f
    };
    unsigned int indices[] = {0, 1, 2, 2, 3, 0};

    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glUseProgram(shaderProgram);
    GLint colorLoc = glGetUniformLocation(shaderProgram, "color");

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw grid
        glUniform4f(colorLoc, 0.2f, 0.2f, 0.2f, 1.0f);
        for (float i = -1.0f; i <= 1.0f; i += 2.0f / GRID_SIZE) {
            float vertices[] = {i, -1.0f, i, 1.0f, -1.0f, i, 1.0f, i};
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glDrawArrays(GL_LINES, 0, 4);
        }

        // Draw units
        for (int i = 0; i < unitCount; i++) {
            float x = units[i].x * 2.0f / GRID_SIZE - 1.0f;
            float y = units[i].y * 2.0f / GRID_SIZE - 1.0f;
            glUniform4f(colorLoc, units[i].selected ? 1.0f : 0.0f, 0.0f, units[i].selected ? 0.0f : 1.0f, 1.0f);

            glBindVertexArray(VAO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);
            glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "transform"), 1, GL_FALSE, 
                (float[]){1.0f, 0.0f, 0.0f, x, 0.0f, 1.0f, 0.0f, y, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f});
            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // Cleanup
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &EBO);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    return 0;
}