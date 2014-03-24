#version 130

in vec4 position;
in vec4 color;

out vec4 fcolor;

void main()
{
    gl_Position = position;
    fcolor = color;
}
