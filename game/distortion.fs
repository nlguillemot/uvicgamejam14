#version 130

in vec2 ftexcoord;

out vec4 color;

uniform sampler2D img;
uniform vec2 LensCenter;
uniform vec2 ScreenCenter;
uniform vec2 Scale;
uniform vec2 ScaleIn;
uniform vec4 HmdWarpParam;

vec2 HmdWarp(vec2 in01)
{
    vec2 r = (in01 - LensCenter) * ScaleIn; // scales to [-1,1]
    float rSq = r.x * r.x + r.y * r.y;
    vec2 f_r = (HmdWarpParam.x +
                HmdWarpParam.y * rSq +
                HmdWarpParam.z * rSq * rSq +
                HmdWarpParam.w * rSq * rSq * rSq);
    return LensCenter + Scale * f_r * r;
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
        color = texture(img, tc);
    }
}

