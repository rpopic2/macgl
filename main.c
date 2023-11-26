#include <stdio.h>
#include <stdlib.h>
#include "glad/glad.h"
#include "GLFW/glfw3.h"

static const char *read_to_string(const char *path, long *length) {
    FILE *file = fopen(path, "r");
    if (!file) {
        printf("could not find file %s\n", path);
        return NULL;
    }
    fseek(file, 0l, SEEK_END);
    long len = ftell(file);
    fseek(file, 0l, SEEK_SET);
    *length = len;

    char *buf = malloc(len);
    if (!buf) {
        printf("malloc failed\n");
        return NULL;
    }
    char *ptr = buf;
    while ((*ptr++ = getc(file)) != EOF) { }
    fclose(file);

    printf("%p", buf);

    return buf;
}

int main(void) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    GLFWwindow *window = glfwCreateWindow(400, 400, "macgl", NULL, NULL);
    glfwMakeContextCurrent(window);

    gladLoadGL();
    printf("%s\n", glGetString(GL_VERSION));
    glViewport(0, 0, 400, 400);

    float vertex[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        0.0f, 0.5f, 0.0f,
    };
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), 0);
    glEnableVertexAttribArray(0);

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    long vlen, flen;
    const char *vert_src = read_to_string("shader.vert", &vlen);
    const char *frag_src = read_to_string("shader.frag", &flen);
    glShaderSource(vert, 1, &vert_src, (const GLint *)&vlen);
    glShaderSource(frag, 1, &frag_src, (const GLint *)&flen);
    glCompileShader(vert);
    glCompileShader(frag);

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    glUseProgram(program);

    glDeleteShader(vert);
    glDeleteShader(frag);
    free((void *)vert_src);
    free((void *)frag_src);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLES, 0, 3);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteProgram(program);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glfwDestroyWindow(window);
    glfwTerminate();
}

