#include <SDL.h>
#include <SOIL2.h>
#include <glm/glm.hpp>
#include <glm/gtx/matrix_operation.hpp>
#include <stdio.h>

#include <SDL2plus.hpp>
#include <GLplus.hpp>
#include <GLmesh.hpp>
#include <tiny_obj_loader.h>

// source code for the vertex shader and fragment shader
static const char* VertexShaderSource =
        "#version 130\n"
        "in vec4 position;\n"
        "uniform mat4 modelview;\n"
        "uniform mat4 projection;\n"
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
        std::shared_ptr<GLplus::Shader> vertexShader = std::make_shared<GLplus::Shader>(GL_VERTEX_SHADER);
        vertexShader->Compile(VertexShaderSource);

        std::shared_ptr<GLplus::Shader> fragmentShader = std::make_shared<GLplus::Shader>(GL_FRAGMENT_SHADER);
        fragmentShader->Compile(FragmentShaderSource);

        // attach & link
        shaderProgram.Attach(vertexShader);
        shaderProgram.Attach(fragmentShader);
        shaderProgram.Link();
    }

    GLmesh::StaticMesh cubeMesh;

    {
        std::vector<tinyobj::shape_t> shapes;
        tinyobj::LoadObj(shapes, "cube.obj");
        if (shapes.empty())
        {
            throw std::runtime_error("Expected shapes.");
        }
        cubeMesh.LoadShape(shapes.front());
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

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        cubeMesh.Render(shaderProgram);

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
        throw;
    }
    catch (...)
    {
        fprintf(stderr, "Fatal exception: (Unknown)\n");
        throw;
    }
}
