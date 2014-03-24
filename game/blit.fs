#version 130

in vec2 ftexcoord;

out vec4 color;

uniform sampler2D img;

void main()
{
    color = texture(img, ftexcoord);
}
