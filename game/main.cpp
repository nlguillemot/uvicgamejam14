#include <SDL.h>
#include <SOIL2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
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
        "void main() {\n"
        "    fnormal = normal;\n"
        "    ftexcoord0 = texcoord0;\n"
        "    gl_Position = projection * modelview * position;\n"
        "}";

static const char* FragmentShaderSource =
        "#version 130\n"
        "out vec4 color;\n"
        "in vec3 fnormal;\n"
        "in vec2 ftexcoord0;\n"
        "uniform sampler2D diffuseTexture;\n"
        "void main() {\n"
        "    color = texture(diffuseTexture, ftexcoord0);\n"
        "}";

struct Scene
{
    GLmesh::StaticMesh mCubeMesh;
    GLplus::Program mObjectShader;

    Scene()
    {
        // compile shaders
        std::shared_ptr<GLplus::Shader> vertexShader = std::make_shared<GLplus::Shader>(GL_VERTEX_SHADER);
        vertexShader->Compile(VertexShaderSource);

        std::shared_ptr<GLplus::Shader> fragmentShader = std::make_shared<GLplus::Shader>(GL_FRAGMENT_SHADER);
        fragmentShader->Compile(FragmentShaderSource);

        // attach & link
        mObjectShader.Attach(vertexShader);
        mObjectShader.Attach(fragmentShader);
        mObjectShader.Link();

        // load box into mesh
        std::vector<tinyobj::shape_t> shapes;
        tinyobj::LoadObj(shapes, "box.obj");
        if (shapes.empty())
        {
            throw std::runtime_error("Expected shapes.");
        }
        mCubeMesh.LoadShape(shapes.front());
    }

    void Render(glm::mat4 projection, glm::mat4 viewAdjustmentForEye) const
    {
        glm::vec3 center(0.0f);
        glm::vec3 eyePoint = glm::vec3(0.0f, 5.0f, 5.0f);
        glm::vec3 up = glm::vec3(0.0f,1.0f,0.0f);

        float rotation = SDL_GetTicks() / 1000.0f * 90.0f;
        glm::mat4 modelview;
        modelview = glm::rotate(modelview, rotation, glm::vec3(0,1,0));
        modelview = glm::lookAt(eyePoint, center, up) * modelview;
        modelview = viewAdjustmentForEye * modelview;

        mObjectShader.UploadMatrix4("modelview", GL_FALSE, &modelview[0][0]);
        mObjectShader.UploadMatrix4("projection", GL_FALSE, &projection[0][0]);

        mCubeMesh.Render(mObjectShader);
    }
};

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

    if (!ovrDevice)
    {
        throw std::runtime_error("Couldn't find device");
    }

    OVR::HMDInfo hmdInfo;
    if (!ovrDevice->GetDeviceInfo(&hmdInfo))
    {
        throw std::runtime_error("Couldn't get device info");
    }

    SDL2plus::LibSDL sdl(SDL_INIT_VIDEO);

    sdl.SetGLAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    sdl.SetGLAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    sdl.SetGLAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // create the window
    SDL2plus::Window window(hmdInfo.HResolution, hmdInfo.VResolution, "Game", SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);

    Scene scene;

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

        glClearColor(1.0f,1.0f,1.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        glEnable(GL_DEPTH_TEST);;

        OVR::Util::Render::StereoConfig stereoConfig;
        stereoConfig.SetHMDInfo(hmdInfo);

        const OVR::Util::Render::StereoEyeParams& leftEyeParams = stereoConfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Left);
        glViewport(0, 0, window.GetWidth() / 2, window.GetHeight());
        glm::mat4 leftEyeProjection = glm::make_mat4((const float*) leftEyeParams.Projection.Transposed().M);
        glm::mat4 leftViewAdjustment = glm::make_mat4((const float*) leftEyeParams.ViewAdjust.Transposed().M);
        scene.Render(leftEyeProjection, leftViewAdjustment);

        const OVR::Util::Render::StereoEyeParams& rightEyeParams = stereoConfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Right);
        glViewport(window.GetWidth() / 2, 0, window.GetWidth() / 2, window.GetHeight());
        glm::mat4 rightEyeProjection = glm::make_mat4((const float*) rightEyeParams.Projection.Transposed().M);
        glm::mat4 rightViewAdjustment = glm::make_mat4((const float*) rightEyeParams.ViewAdjust.Transposed().M);
        scene.Render(rightEyeProjection, rightViewAdjustment);

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
