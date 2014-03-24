#include <SDL.h>
#include <SOIL2.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>

#include <sstream>
#include <fstream>

#include <SDL2plus.hpp>
#include <GLplus.hpp>
#include <GLmesh.hpp>
#include <tiny_obj_loader.h>

#include <OVR.h>

class Scene
{
    GLmesh::StaticMesh mCubeMesh;
    GLplus::Program mObjectShader;

public:
    Scene()
        : mObjectShader(GLplus::Program::FromFiles("object.vs","object.fs"))
    {
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
        glm::vec3 up = glm::vec3(0.0f,1.0f,0.0f);

        float rotation = SDL_GetTicks() / 1000.0f * 90.0f;
        float rotation2 = SDL_GetTicks() / 1000.0f * 3.14 / 2;
        float rotation3 = SDL_GetTicks() / 1000.0f * 3.14 / 3;

        glm::vec3 eyePoint = glm::vec3(0.0f, 5.0f * sin(rotation2), 5.0f * fabs(sin(rotation3) + 1.5f));

        glm::mat4 modelview;
        modelview = glm::rotate(modelview, rotation, glm::vec3(0,1,0));
        modelview = glm::lookAt(eyePoint, center, up) * modelview;
        modelview = viewAdjustmentForEye * modelview;

        mObjectShader.UploadMatrix4("modelview", GL_FALSE, &modelview[0][0]);
        mObjectShader.UploadMatrix4("projection", GL_FALSE, &projection[0][0]);

        mCubeMesh.Render(mObjectShader);
    }
};

class Oculus
{
    OVR::System mSystem;
    std::unique_ptr<OVR::DeviceManager, void(*)(OVR::DeviceManager*)> mDeviceManager;
    std::unique_ptr<OVR::HMDDevice, void(*)(OVR::HMDDevice*)> mHMDDevice;

public:
    Oculus()
        : mDeviceManager(OVR::DeviceManager::Create(),
                         [](OVR::DeviceManager* manager){ if (manager) manager->Release(); })
        , mHMDDevice(mDeviceManager ? mDeviceManager->EnumerateDevices<OVR::HMDDevice>().CreateDevice() : nullptr,
                     [](OVR::HMDDevice* device){ if (device) device->Release(); })
    {
        if (!mDeviceManager || !mHMDDevice)
        {
            printf("Warning: Couldn't connect to real oculus. Using fake oculus.\n");
        }
    }

    OVR::HMDInfo GetHMDInfo() const
    {
        OVR::HMDInfo info;
        if (mHMDDevice)
        {
            mHMDDevice->GetDeviceInfo(&info);
        }
        else
        {
            info.HResolution = 1280;
            info.VResolution = 800;
            info.HScreenSize = 0.14976f;
            info.VScreenSize = 0.09356f;
            info.VScreenCenter = 0.0468f;
            info.EyeToScreenDistance = 0.041f;
            info.LensSeparationDistance = 0.0635f;
            info.InterpupillaryDistance = 0.064f;
            info.DistortionK[0] = 1.0f;
            info.DistortionK[1] = 0.22f;
            info.DistortionK[2] = 0.24f;
            info.DistortionK[3] = 0.0f;
            info.ChromaAbCorrection[0] = 0.996f;
            info.ChromaAbCorrection[1] = -0.004f;
            info.ChromaAbCorrection[2] = 1.014f;
            info.ChromaAbCorrection[3] = 0.0f;
        }
        return info;
    }
};

void run()
{
    Oculus oculus;
    const OVR::HMDInfo hmdInfo = oculus.GetHMDInfo();

    SDL2plus::LibSDL sdl(SDL_INIT_VIDEO);

    sdl.SetGLAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    sdl.SetGLAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    sdl.SetGLAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // create the window
    SDL2plus::Window window(hmdInfo.HResolution, hmdInfo.VResolution, "Game", SDL_WINDOW_OPENGL | SDL_WINDOW_FULLSCREEN);

    Scene scene;

    std::shared_ptr<GLplus::Texture2D> renderedTexture = std::make_shared<GLplus::Texture2D>();
    renderedTexture->CreateStorage(1, GL_RGBA8, hmdInfo.HResolution, hmdInfo.VResolution);

    std::shared_ptr<GLplus::RenderBuffer> depthBuffer = std::make_shared<GLplus::RenderBuffer>();
    depthBuffer->CreateStorage(GL_DEPTH_COMPONENT16, hmdInfo.HResolution, hmdInfo.VResolution);

    GLplus::FrameBuffer offscreenFrameBuffer;
    offscreenFrameBuffer.Attach(GL_COLOR_ATTACHMENT0, renderedTexture);
    offscreenFrameBuffer.Attach(GL_DEPTH_ATTACHMENT, depthBuffer);
    offscreenFrameBuffer.ValidateStatus();

    GLplus::Program distortionProgram(GLplus::Program::FromFiles("distortion.vs","distortion.fs"));
    GLplus::Program blitProgram(GLplus::Program::FromFiles("blit.vs","blit.fs"));

    std::shared_ptr<GLplus::Buffer> fullscreenPositions, fullscreenTexcoords;
    {
        static const float distortionPositionsData[] = {
            // bottom triangle of left eye
            -1.0f,  1.0f,
            -1.0f, -1.0f,
             0.0f, -1.0f,
            // top triangle of left eye
             0.0f, -1.0f,
             0.0f,  1.0f,
            -1.0f,  1.0f,
            // bottom triangle of right eye
             0.0f,  1.0f,
             0.0f, -1.0f,
             1.0f, -1.0f,
            // top triangle of right eye
             1.0f, -1.0f,
             1.0f,  1.0f,
             0.0f,  1.0f
        };

        static const float distortionTexcoordsData[] = {
            // bottom triangle of left eye
            0.0f, 1.0f,
            0.0f, 0.0f,
            0.5f, 0.0f,
            // top triangle of left eye
            0.5f, 0.0f,
            0.5f, 1.0f,
            0.0f, 1.0f,
            // bottom triangle of right eye
            0.5f, 1.0f,
            0.5f, 0.0f,
            1.0f, 0.0f,
            // top triangle of right eye
            1.0f, 0.0f,
            1.0f, 1.0f,
            0.5f, 1.0f
        };

        fullscreenPositions = std::make_shared<GLplus::Buffer>(GL_ARRAY_BUFFER);
        fullscreenPositions->Upload(sizeof(distortionPositionsData), distortionPositionsData, GL_STATIC_DRAW);

        fullscreenTexcoords = std::make_shared<GLplus::Buffer>(GL_ARRAY_BUFFER);
        fullscreenTexcoords->Upload(sizeof(distortionTexcoordsData), distortionTexcoordsData, GL_STATIC_DRAW);
    }

    OVR::Util::Render::StereoConfig stereoConfig;
    stereoConfig.SetFullViewport(OVR::Util::Render::Viewport(0, 0, window.GetWidth(), window.GetHeight()));
    stereoConfig.SetStereoMode(OVR::Util::Render::Stereo_LeftRight_Multipass);
    stereoConfig.SetHMDInfo(hmdInfo);
    stereoConfig.SetDistortionFitPointVP(-1.0f, 0.0f);

    const OVR::Util::Render::StereoEyeParams& leftEyeParams = stereoConfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Left);
    const OVR::Util::Render::StereoEyeParams& rightEyeParams = stereoConfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Right);

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

        {
            GLplus::ScopedFrameBufferBind offscreenBind(offscreenFrameBuffer);

            glClearColor(1.0f,1.0f,1.0f,1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glEnable(GL_DEPTH_TEST);
            GLplus::CheckGLErrors();

            glViewport(0, 0, window.GetWidth() / 2, window.GetHeight());
            glm::mat4 leftEyeProjection = glm::make_mat4((const float*) leftEyeParams.Projection.Transposed().M);
            glm::mat4 leftViewAdjustment = glm::make_mat4((const float*) leftEyeParams.ViewAdjust.Transposed().M);
            scene.Render(leftEyeProjection, leftViewAdjustment);

            glViewport(window.GetWidth() / 2, 0, window.GetWidth() / 2, window.GetHeight());
            glm::mat4 rightEyeProjection = glm::make_mat4((const float*) rightEyeParams.Projection.Transposed().M);
            glm::mat4 rightViewAdjustment = glm::make_mat4((const float*) rightEyeParams.ViewAdjust.Transposed().M);
            scene.Render(rightEyeProjection, rightViewAdjustment);
        }

        {
            bool useDistortion = true;

            glViewport(0, 0, window.GetWidth(), window.GetHeight());
            glClearColor(1.0f,1.0f,1.0f,1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
            glDisable(GL_DEPTH_TEST);
            GLplus::CheckGLErrors();

            const float distortionScale = stereoConfig.GetDistortionScale();
            const float lensCenter = 0.5f - hmdInfo.LensSeparationDistance / 2 / hmdInfo.HScreenSize;

            GLplus::Program* fullscreenProgram(nullptr);

            if (useDistortion)
            {
                distortionProgram.UploadUint("img", 0);
                distortionProgram.UploadVec2("Scale", 0.5f - lensCenter, 0.5f);
                distortionProgram.UploadVec2("ScaleIn", 1.0f / (0.5f - lensCenter), 2.0f);
                distortionProgram.UploadVec4("HmdWarpParam", hmdInfo.DistortionK);
                distortionProgram.UploadVec2("PositionScale", 1.0f, 1.0f);

                fullscreenProgram = &distortionProgram;
            }
            else
            {
                blitProgram.UploadUint("img", 0);
                fullscreenProgram = &blitProgram;
            }

            GLplus::VertexArray fullscreenMesh;
            fullscreenMesh.SetAttribute(fullscreenProgram->GetAttributeLocation("texcoord"),
                                        fullscreenTexcoords, 2, GL_FLOAT, GL_FALSE, 0, 0);
            fullscreenMesh.SetAttribute(fullscreenProgram->GetAttributeLocation("position"),
                                        fullscreenPositions, 2, GL_FLOAT, GL_FALSE, 0, 0);

            GLplus::ScopedTextureBind textureBind(*renderedTexture, GL_TEXTURE0);
            GLplus::ScopedVertexArrayBind vertexArrayBind(fullscreenMesh);
            GLplus::ScopedProgramBind programBind(*fullscreenProgram);

            // draw left eye
            if (useDistortion)
            {
                distortionProgram.UploadVec2("LensCenter", lensCenter, 0.5f);
                distortionProgram.UploadVec2("ScreenCenter", 0.25f, 0.5f);
            }
            GLplus::DrawArrays(GL_TRIANGLES, 0, 6);

            // draw right eye
            if (useDistortion)
            {
                distortionProgram.UploadVec2("LensCenter", 1.0f - lensCenter, 0.5f);
                distortionProgram.UploadVec2("ScreenCenter", 0.75f, 0.5f);
            }
            GLplus::DrawArrays(GL_TRIANGLES, 6, 6);
        }

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
