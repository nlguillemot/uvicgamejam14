#version 130

in vec2 ftexcoord;

out vec4 color;

uniform sampler2D RenderedStereoscopicScene;

void main()
{
    color = texture(RenderedStereoscopicScene, ftexcoord);
}
