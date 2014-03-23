#version 130

in vec4 position;
in vec3 normal;
in vec2 texcoord0;

out vec3 fnormal;
out vec2 ftexcoord0;

uniform mat4 modelview;
uniform mat4 projection;

void main()
{
    fnormal = normal;
    ftexcoord0 = texcoord0;
    gl_Position = projection * modelview * position;
}
