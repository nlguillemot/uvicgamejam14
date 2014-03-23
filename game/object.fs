#version 130

in vec3 fnormal;
in vec2 ftexcoord0;

out vec4 color;

uniform sampler2D diffuseTexture;

void main()
{
    color = texture(diffuseTexture, ftexcoord0);
}
