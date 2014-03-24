#version 130

in vec2 ftexcoord;

out vec4 color;

// frame of video to render with a barrel distortion in two draw calls
uniform sampler2D RenderedStereoscopicScene;

// position of lens center in texture coordinates
uniform vec2 LensCenter;

// center of the screen in texture coordinates
uniform vec2 ScreenCenter;

// ratio to scale half-screen texture coordinates
// to coordinates in a unit space around the lens
uniform vec2 TextureToLensScale;
// and vice versa
uniform vec2 LensToTextureScale;

// the four distortion parameters
uniform vec4 HmdWarpParam;

// converts the texture coordinate to a barrel distorted one
vec2 HmdWarp(vec2 rawTexcoord)
{
    // convert texture coordinates from [0,1] to [-1,1]
    vec2 inLensSpace = (rawTexcoord - LensCenter) * TextureToLensScale; // scales to [-1,1]

	// calculate distortion
    float rSq = inLensSpace.x * inLensSpace.x + inLensSpace.y * inLensSpace.y;
    float distortionScale = HmdWarpParam.x +
                            HmdWarpParam.y * rSq +
                            HmdWarpParam.z * rSq * rSq +
                            HmdWarpParam.w * rSq * rSq * rSq;

	// translate back to texture coordinate space
    return LensCenter + inLensSpace * distortionScale * LensToTextureScale;
}

void main()
{
    vec2 tc = HmdWarp(ftexcoord);

    if (any(bvec2(clamp(tc, ScreenCenter-vec2(0.25,0.5), ScreenCenter+vec2(0.25,0.5)) - tc)))
    {
       color = vec4(0);
    }
    else
    {
        color = texture(RenderedStereoscopicScene, tc);
    }
}

