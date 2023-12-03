#include <stdio.h>
#include <stdlib.h>
#include <png.h>
#include "glad/glad.h"
#include <GLFW/glfw3.h>

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

    return buf;
}

static void print_tex_data(unsigned char *tex_buf,
        unsigned int img_width, unsigned int img_height,
        unsigned char num_col_ch) {
    printf("width: %d, height: %d\n", img_width, img_height);
    size_t img_size = img_height * img_height * num_col_ch;
    size_t row_size = img_width * num_col_ch;
    for (size_t i = 0; i < img_size; ++i) {
        if (i % row_size == 0)
            printf("\n");
        if (i % num_col_ch == 0)
            printf("\t");
        printf("%02x ", tex_buf[i]);
    }
    printf("\n");
}

static unsigned char *read_png(const char *path,
        unsigned int *width, unsigned int *height, unsigned char *channels) {
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;

    FILE *fp = fopen(path, "rb");
    if (!fp) {
        printf("failed to open file %s\n", path);
        return NULL;
    }
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
            NULL, NULL, NULL);
    if (!png_ptr) {
        printf("failed to create read struct");
        fclose(fp);
        return NULL;
    }
    info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        printf("failed to create info struct\n");
        png_destroy_read_struct(&png_ptr, NULL, NULL);
        fclose(fp);
        return NULL;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        printf("an error occured\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }
    png_init_io(png_ptr, fp);

    png_read_info(png_ptr, info_ptr);
    unsigned int w = png_get_image_width(png_ptr, info_ptr);
    unsigned int h = png_get_image_height(png_ptr, info_ptr);
    unsigned char ch = png_get_channels(png_ptr, info_ptr);
    *height = h;
    *width = w;
    *channels = ch;

    png_bytep buf = malloc(w * h * ch);
    if (!buf) {
        printf("malloc failed\n");
        png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
        fclose(fp);
        return NULL;
    }
    size_t row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    for (size_t i = h; i-- > 0;) {
        png_read_row(png_ptr, buf + (i * row_bytes), NULL);
    }
    png_read_end(png_ptr, info_ptr);

    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    fclose(fp);

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
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.0f, 1.0f, 1.0f,
        -0.5f, 0.5f, 0.0f, 0.0f, 1.0f
    };
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex), vertex, GL_STATIC_DRAW);

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void *)(3 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    GLuint indicies[] = {
        0, 1, 2,
        2, 3, 0,
    };
    GLuint ebo;
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indicies), indicies, GL_STATIC_DRAW);

    GLuint vert = glCreateShader(GL_VERTEX_SHADER);
    GLuint frag = glCreateShader(GL_FRAGMENT_SHADER);
    long vlen, flen;
    const char *vert_src = read_to_string("shader.vert", &vlen);
    const char *frag_src = read_to_string("shader.frag", &flen);
    glShaderSource(vert, 1, &vert_src, (GLint *)&vlen);
    glShaderSource(frag, 1, &frag_src, (GLint *)&flen);
    glCompileShader(vert);
    glCompileShader(frag);

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    glDeleteShader(vert);
    glDeleteShader(frag);
    free((void *)vert_src);
    free((void *)frag_src);

    unsigned int img_width, img_height;
    unsigned char num_ch;

    unsigned char *tex_buf = read_png("./pop_cat.png", &img_width, &img_height, &num_ch);
    // print_tex_data(tex_buf, img_width, img_height, num_ch);

    GLuint tex;
    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    GLuint internal_format = GL_RGBA;
    if (num_ch == 3)
        internal_format = GL_RGB;
    glTexImage2D(GL_TEXTURE_2D, 0, internal_format, img_width, img_height,
            0, internal_format, GL_UNSIGNED_BYTE, tex_buf);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    free(tex_buf);

    glUseProgram(program);
    GLint tex_loc = glGetUniformLocation(program, "tex");
    glUniform1i(tex_loc, 0);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteTextures(1, &tex);
    glDeleteProgram(program);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
    glfwDestroyWindow(window);
    glfwTerminate();
}

