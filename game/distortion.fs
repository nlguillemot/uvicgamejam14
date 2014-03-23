#version 130

in vec2 ftexcoord;

uniform sampler2D img;
uniform vec2 LensCenter;
uniform vec2 ScreenCenter;
uniform vec2 Scale;
uniform vec2 ScaleIn;
uniform vec4 HmdWarpParam;

vec2 HmdWarp(vec2 in01)
{
    vec2 theta = (in01 - LensCenter) * ScaleIn; // Scales to [-1, 1]
    float rSq = theta.x * theta.x + theta.y * theta.y;
    vec2 rvector = theta * (HmdWarpParam.x + HmdWarpParam.y * rSq +
                            HmdWarpParam.z * rSq * rSq +
                            HmdWarpParam.w * rSq * rSq * rSq);
    return LensCenter + Scale * rvector;
}

void main()
{
    vec2 tc = HmdWarp(ftexcoord);
    if (any(clamp(tc, ScreenCenter-vec2(0.25,0.5),
                      ScreenCenter+vec2(0.25,0.5)) - tc))
       return vec4(0);
    return texture(img, ftexcoord);
}

