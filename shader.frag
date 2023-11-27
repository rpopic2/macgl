#version 410 core

uniform sampler2D tex;

in vec2 v_tex_coord;

out vec4 frag_color;

void main() {
    frag_color = texture(tex, v_tex_coord);
}

