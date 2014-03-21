#include <SDL.h>
#include <SOIL2.h>

#include <stdio.h>

#include <SDL2plus.hpp>
#include <GLplus.hpp>

// a boring triangle
static const GLfloat TriangleData[] = {
     1,-1,-1,
    -1,-1,-1,
     1, 1,-1
};

// source code for the vertex shader and fragment shader
static const char* VertexShaderSource =
        "#version 130\n"
        "in vec4 position;\n"
        "void main() {\n"
        "    gl_Position = position;\n"
        "}";

static const char* FragmentShaderSource =
        "#version 130\n"
        "out vec4 color;\n"
        "void main() {\n"
        "    color = vec4(1,0,0,1);\n"
        "}";

void run()
{
    SDL2plus::LibSDL sdl(SDL_INIT_VIDEO);

    sdl.SetGLAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    sdl.SetGLAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    sdl.SetGLAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // create the window
    SDL2plus::Window window(640, 480, "Game", SDL_WINDOW_OPENGL);

    // link shaders into a program
    GLplus::Program shaderProgram;
    {
        // compile shaders
        GLplus::Shader vertexShader(GL_VERTEX_SHADER);
        vertexShader.Compile(VertexShaderSource);

        GLplus::Shader fragmentShader(GL_FRAGMENT_SHADER);
        fragmentShader.Compile(FragmentShaderSource);

        // attach & link
        shaderProgram.Attach(vertexShader);
        shaderProgram.Attach(fragmentShader);
        shaderProgram.Link();
    }

    // create a vertex buffer object to store the vertices
    GLplus::VertexBuffer positionVertexData(GL_ARRAY_BUFFER);
    positionVertexData.Upload(sizeof(TriangleData), TriangleData, GL_STATIC_DRAW);

    // create a VAO to hold the model
    GLplus::VertexArray model;
    {
        // get positions of attributes
        GLint positionAttributeLocation = shaderProgram.GetAttributeLocation("position");

        // specify the layout of the position attribute
        model.SetAttribute(positionAttributeLocation, positionVertexData, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    // begin main loop
    int isGameRunning = 1;
    while (isGameRunning)
    {
        // handle all the events
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            // if the window is being requested to close, then stop the game.
            if (e.type == SDL_QUIT) {
                isGameRunning = 0;
            }
        }

        glClear(GL_COLOR_BUFFER_BIT);

        DrawArrays(shaderProgram, model, GL_TRIANGLES, 0, 3);

        // flip the display
        window.GLSwapWindow();

        // throttle the frame rate to 60fps
        SDL_Delay(1000/60);
    }
}

int main(int argc, char *argv[])
{
    try
    {
        run();
    }
    catch (const std::exception& e)
    {
        fprintf(stderr, "Fatal exception: %s\n", e.what());
        return -1;
    }
    catch (...)
    {
        fprintf(stderr, "Fatal exception: (Unknown)\n");
        throw;
    }
}
