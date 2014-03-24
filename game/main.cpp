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

class OverlayDebugLines
{
    std::shared_ptr<GLplus::Buffer> mVertexBuffer;
    size_t mNumVertices;

public:
    OverlayDebugLines(const OVR::HMDInfo& info)
    {
        float vcenter = 1.0f - info.VScreenCenter / info.VScreenSize * 2.0f;

        std::vector<float> vertData;

        vertData.insert(vertData.end(), {
            // vertical line in the middle
            0.0f, 1.0f,         1.0f, 0.0f, 0.0f, 1.0f,
            0.0f,-1.0f,         1.0f, 0.0f, 0.0f, 1.0f,
            // horizontal line at the vertical center of the oculus
           -1.0f, vcenter,      1.0f, 0.0f, 0.0f, 1.0f,
            1.0f, vcenter,      1.0f, 0.0f, 0.0f, 1.0f,
        });

        float aspect = info.HScreenSize / info.VScreenSize;
        float lensFromCenter = info.LensSeparationDistance / info.HScreenSize;
        float lensRadius = 0.2f;
        float eyeFromCenter = info.InterpupillaryDistance / info.HScreenSize;
        float eyeRadius = 0.13f;

        float lensCenters[2] = { -lensFromCenter, lensFromCenter };
        float eyeCenters[2]  = { -eyeFromCenter,  eyeFromCenter  };

        vertData.insert(vertData.end(), {
            // left lens vertical line
            lensCenters[0], 1.0f,      0.0f, 1.0f, 0.0f, 1.0f,
            lensCenters[0],-1.0f,      0.0f, 1.0f, 0.0f, 1.0f,
            // right lens vertical line
            lensCenters[1], 1.0f,      0.0f, 1.0f, 0.0f, 1.0f,
            lensCenters[1],-1.0f,      0.0f, 1.0f, 0.0f, 1.0f,
        });

        int numCircleVerts = 30;
        for (int eye = 0; eye < 2; eye++)
        {
            for (int i = 0; i < numCircleVerts; i++)
            {
                float startAngle = i * 2 * glm::pi<float>() / numCircleVerts;
                float endAngle = (i+1) * 2 * glm::pi<float>() / numCircleVerts;

                float x1 = lensCenters[eye] + lensRadius * cos(startAngle);
                float x2 = lensCenters[eye] + lensRadius * cos(endAngle);
                float y1 = vcenter + lensRadius * sin(startAngle) * aspect;
                float y2 = vcenter + lensRadius * sin(endAngle) * aspect;

                vertData.insert(vertData.end(), {
                    x1, y1,     1.0f, 0.0f, 0.0f, 1.0f,
                    x2, y2,     1.0f, 0.0f, 0.0f, 1.0f
                });

                x1 = eyeCenters[eye] + eyeRadius * cos(startAngle) * 1.3f;
                x2 = eyeCenters[eye] + eyeRadius * cos(endAngle)   * 1.3f;
                y1 = vcenter + eyeRadius * sin(startAngle) * aspect * 0.8f;
                y2 = vcenter + eyeRadius * sin(endAngle) * aspect   * 0.8f;

                vertData.insert(vertData.end(), {
                    x1, y1,     0.0f, 1.0f, 0.0f, 1.0f,
                    x2, y2,     0.0f, 1.0f, 0.0f, 1.0f
                });

            }
        }

        mVertexBuffer = std::make_shared<GLplus::Buffer>(GL_ARRAY_BUFFER);
        mVertexBuffer->Upload(vertData.size() * sizeof(vertData[0]), vertData.data(), GL_STATIC_DRAW);
        mNumVertices = vertData.size();
    }

    void Render(const GLplus::Program& program)
    {
        GLplus::VertexArray vertexArray;

        GLint positionLoc;
        if (program.TryGetAttributeLocation("position", positionLoc))
        {
            vertexArray.SetAttribute(positionLoc, mVertexBuffer,
                2, GL_FLOAT, GL_FALSE, sizeof(float) * 6, 0);
        }

        GLint colorLoc;
        if (program.TryGetAttributeLocation("color", colorLoc))
        {
            vertexArray.SetAttribute(colorLoc, mVertexBuffer,
                4, GL_FLOAT, GL_FALSE, sizeof(float) * 6, sizeof(float) * 2);
        }

        GLplus::ScopedVertexArrayBind vertexArrayBind(vertexArray);
        GLplus::ScopedProgramBind programBind(program);

        GLplus::DrawArrays(GL_LINES, 0, mNumVertices);
    }
};

OverlayDebugLines GetDebugLineVertices(const OVR::HMDInfo& info)
{
}

void run()
{
    Oculus oculus;
    const OVR::HMDInfo hmdInfo = oculus.GetHMDInfo();

    SDL2plus::LibSDL sdl(SDL_INIT_VIDEO);

    sdl.SetGLAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    sdl.SetGLAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    sdl.SetGLAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    // create the window
    SDL2plus::Window window(hmdInfo.HResolution, hmdInfo.VResolution, "Game", SDL_WINDOW_OPENGL ); // | SDL_WINDOW_FULLSCREEN);

    Scene scene;

    std::shared_ptr<GLplus::Texture2D> renderedTexture = std::make_shared<GLplus::Texture2D>();
    renderedTexture->CreateStorage(1, GL_RGBA8, hmdInfo.HResolution, hmdInfo.VResolution);

    std::shared_ptr<GLplus::RenderBuffer> depthBuffer = std::make_shared<GLplus::RenderBuffer>();
    depthBuffer->CreateStorage(GL_DEPTH_COMPONENT16, hmdInfo.HResolution, hmdInfo.VResolution);

    GLplus::FrameBuffer offscreenFrameBuffer;
    offscreenFrameBuffer.Attach(GL_COLOR_ATTACHMENT0, renderedTexture);
    offscreenFrameBuffer.Attach(GL_DEPTH_ATTACHMENT, depthBuffer);
    offscreenFrameBuffer.ValidateStatus();

    GLplus::Program barrelProgram(GLplus::Program::FromFiles("barrel.vs","barrel.fs"));
    GLplus::Program blitProgram(GLplus::Program::FromFiles("blit.vs","blit.fs"));
    GLplus::Program debugLineProgram(GLplus::Program::FromFiles("debugline.vs","debugline.fs"));

    std::shared_ptr<GLplus::Buffer> twoHalfScreenPositions, twoHalfScreenTexcoords;
    {
        static const float twoHalfScreenPositionsData[] = {
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

        static const float twoHalfScreenTexcoordsData[] = {
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

        twoHalfScreenPositions = std::make_shared<GLplus::Buffer>(GL_ARRAY_BUFFER);
        twoHalfScreenPositions->Upload(sizeof(twoHalfScreenPositionsData), twoHalfScreenPositionsData, GL_STATIC_DRAW);

        twoHalfScreenTexcoords = std::make_shared<GLplus::Buffer>(GL_ARRAY_BUFFER);
        twoHalfScreenTexcoords->Upload(sizeof(twoHalfScreenTexcoordsData), twoHalfScreenTexcoordsData, GL_STATIC_DRAW);
    }

    OverlayDebugLines debugLines(hmdInfo);

    OVR::Util::Render::StereoConfig stereoConfig;
    stereoConfig.SetFullViewport(OVR::Util::Render::Viewport(0, 0, window.GetWidth(), window.GetHeight()));
    stereoConfig.SetStereoMode(OVR::Util::Render::Stereo_LeftRight_Multipass);
    stereoConfig.SetHMDInfo(hmdInfo);
    stereoConfig.SetDistortionFitPointVP(-1.0f, 0.0f);

    const OVR::Util::Render::StereoEyeParams& leftEyeParams = stereoConfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Left);
    const OVR::Util::Render::StereoEyeParams& rightEyeParams = stereoConfig.GetEyeRenderParams(OVR::Util::Render::StereoEye_Right);

    Uint32 timeOfLastFrame = SDL_GetTicks();

    // begin main loop
    int isGameRunning = 1;
    while (isGameRunning)
    {
        Uint32 timeOfThisFrame = SDL_GetTicks();
        Uint32 deltaTimeMilliSec = timeOfThisFrame - timeOfLastFrame;

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
            const float screenLeftToLeftLensCenter = 0.5f - hmdInfo.LensSeparationDistance / 2 / hmdInfo.HScreenSize;

            GLplus::Program* fullscreenProgram(nullptr);

            if (useDistortion)
            {
                glm::vec2 lensSpaceToTextureSpaceScale(0.5f - screenLeftToLeftLensCenter, 0.5f);
                glm::vec2 textureSpaceToLensSpaceScale(1.0f / lensSpaceToTextureSpaceScale);

                barrelProgram.UploadUint("img", 0);
                barrelProgram.UploadVec2("Scale", &lensSpaceToTextureSpaceScale[0]);
                barrelProgram.UploadVec2("ScaleIn", &textureSpaceToLensSpaceScale[0]);
                barrelProgram.UploadVec4("HmdWarpParam", hmdInfo.DistortionK);
                barrelProgram.UploadVec2("PositionScale", 1.0f, 1.0f);

                fullscreenProgram = &barrelProgram;
            }
            else
            {
                blitProgram.UploadUint("img", 0);
                fullscreenProgram = &blitProgram;
            }

            GLplus::VertexArray fullscreenMesh;
            fullscreenMesh.SetAttribute(fullscreenProgram->GetAttributeLocation("texcoord"),
                                        twoHalfScreenTexcoords, 2, GL_FLOAT, GL_FALSE, 0, 0);
            fullscreenMesh.SetAttribute(fullscreenProgram->GetAttributeLocation("position"),
                                        twoHalfScreenPositions, 2, GL_FLOAT, GL_FALSE, 0, 0);

            GLplus::ScopedTextureBind textureBind(*renderedTexture, GL_TEXTURE0);
            GLplus::ScopedVertexArrayBind vertexArrayBind(fullscreenMesh);
            GLplus::ScopedProgramBind programBind(*fullscreenProgram);

            // draw left eye
            if (useDistortion)
            {
                barrelProgram.UploadVec2("LensCenter", screenLeftToLeftLensCenter, 0.5f);
                barrelProgram.UploadVec2("ScreenCenter", 0.25f, 0.5f);
            }
            GLplus::DrawArrays(GL_TRIANGLES, 0, 6);

            // draw right eye
            if (useDistortion)
            {
                barrelProgram.UploadVec2("LensCenter", 1.0f - screenLeftToLeftLensCenter, 0.5f);
                barrelProgram.UploadVec2("ScreenCenter", 0.75f, 0.5f);
            }
            GLplus::DrawArrays(GL_TRIANGLES, 6, 6);
        }

        debugLines.Render(debugLineProgram);

        // flip the display
        window.GLSwapWindow();

        // throttle the frame rate to 60fps
        if (deltaTimeMilliSec < 1000/60)
        {
            SDL_Delay(1000/60 - deltaTimeMilliSec);
        }

        timeOfLastFrame = timeOfThisFrame;
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
