#include <SDL.h>
#include <SOIL2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>

#include <SDL2plus.hpp>
#include <GLplus.hpp>
#include <GLmesh.hpp>
#include <tiny_obj_loader.h>

#include <OVR.h>

// source code for the vertex shader and fragment shader
static const char* VertexShaderSource =
        "#version 130\n"
        "in vec4 position;\n"
        "in vec3 normal;\n"
        "in vec2 texcoord0;\n"
        "out vec3 fnormal;\n"
        "out vec2 ftexcoord0;\n"
        "uniform mat4 modelview;\n"
        "uniform mat4 projection;\n"
        "uniform vec4 offset;\n"
        "void main() {\n"
        "    fnormal = normal;\n"
        "    ftexcoord0 = texcoord0;\n"
        "    vec4 pos = projection * modelview * position;\n"
        "    gl_Position = offset * pos.w + pos;\n"
        "}";

static const char* FragmentShaderSource =
        "#version 130\n"
        "out vec4 color;\n"
        "in vec3 fnormal;\n"
        "in vec2 ftexcoord0;\n"
        "uniform sampler2D diffuseTexture;\n"
        "void main() {\n"
        "    color = texture(diffuseTexture, ftexcoord0);\n"
        "    // color = vec4((fnormal + vec4(1))/2,1);\n"
        "}";

void run()
{
    OVR::System ovrSystem;

    std::unique_ptr<OVR::DeviceManager, void(*)(OVR::DeviceManager*)>
            ovrDeviceManager(OVR::DeviceManager::Create(),
            [](OVR::DeviceManager* manager){ manager->Release(); });

    if (!ovrDeviceManager)
    {
        throw std::runtime_error("OVR::DeviceManager::Create");
    }

    std::unique_ptr<OVR::HMDDevice, void(*)(OVR::HMDDevice*)>
        ovrDevice(ovrDeviceManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice(),
        [](OVR::HMDDevice* device){ device->Release(); });

    SDL2plus::LibSDL sdl(SDL_INIT_VIDEO);

    sdl.SetGLAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    sdl.SetGLAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    sdl.SetGLAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // create the window
    SDL2plus::Window window(1280, 800, "Game", SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);

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
        tinyobj::LoadObj(shapes, "box.obj");
        if (shapes.empty())
        {
            throw std::runtime_error("Expected shapes.");
        }
        cubeMesh.LoadShape(shapes.front());
    }

    glEnable(GL_DEPTH_TEST);

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

        auto render = [&](glm::vec4 normalizedOffset, float noseToEyeDistance, float rotation) {
            glm::vec3 center(0.0f);
            glm::vec3 eyePoint = glm::vec3(0.0f, 5.0f, 5.0f);
            glm::vec3 up = glm::vec3(0.0f,1.0f,0.0f);
            glm::vec3 eyeToCenter = glm::normalize(center - eyePoint);
            glm::vec3 across = glm::normalize(glm::cross(eyeToCenter,up));
            eyePoint += across * noseToEyeDistance;

            glm::mat4 worldview = glm::lookAt(eyePoint, center, up);
            glm::mat4 scaleM = glm::scale(glm::mat4(), glm::vec3(1.0f,1.0f,1.0f));
            glm::mat4 rotM = glm::rotate(glm::mat4(), rotation, glm::vec3(0.0f,1.0f,0.0f));
            glm::mat4 modelview = rotM * scaleM;
            modelview = worldview * modelview;

            glm::mat4 projection = glm::perspective(90.0f, (float) window.GetWidth() / 2 / window.GetHeight(), 0.1f, 100.0f);

            shaderProgram.UploadMatrix4("modelview", GL_FALSE, &modelview[0][0]);
            shaderProgram.UploadMatrix4("projection", GL_FALSE, &projection[0][0]);
            shaderProgram.UploadVec4("offset", &normalizedOffset[0]);

            cubeMesh.Render(shaderProgram);
        };

        glClearColor(1.0f,1.0f,1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        static float noseToEyeDistance = 0.13f;
        const float noseToEyeDistanceDelta = 0.01f;

        const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);
        if (keyboardState[SDL_SCANCODE_LEFT])
        {
            noseToEyeDistance -= noseToEyeDistanceDelta;
        }
        if (keyboardState[SDL_SCANCODE_RIGHT])
        {
            noseToEyeDistance += noseToEyeDistanceDelta;
        }

        float rotation = SDL_GetTicks() / 1000.0f * 90.0f;
        glViewport(0, 0, window.GetWidth() / 2, window.GetHeight());
        render(glm::vec4(0.15f,0.0f,0.0f,0.0f), -noseToEyeDistance, rotation);

        glViewport(window.GetWidth() / 2, 0, window.GetWidth() / 2, window.GetHeight());
        render(glm::vec4(-0.15f,0.0f,0.0f,0.0f), noseToEyeDistance, rotation);

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
