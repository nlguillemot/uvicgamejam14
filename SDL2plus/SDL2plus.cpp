#include "SDL2plus.hpp"
#include <GL/glew.h>

#include <stdexcept>

namespace SDL2plus
{

LibSDL::LibSDL(Uint32 flags)
{
    if (SDL_Init(flags))
    {
        throw std::runtime_error(SDL_GetError());
    }
}

LibSDL::~LibSDL()
{
    SDL_Quit();
}

void LibSDL::SetGLAttribute(SDL_GLattr attr, int value)
{
    if (SDL_GL_SetAttribute(attr, value))
    {
        throw std::runtime_error(SDL_GetError());
    }
}

void Window::WindowDeleter::operator()(SDL_Window* window) const
{
    SDL_DestroyWindow(window);
}

void Window::GLContextDeleter::operator()(SDL_GLContext context) const
{
    SDL_GL_DeleteContext(context);
}

Window::Window(int width, int height, const char *title, Uint32 flags)
{
    mWindowHandle.reset(
                SDL_CreateWindow(
                    title,
                    SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                    width, height, flags));

    if (!mWindowHandle)
    {
        throw std::runtime_error(SDL_GetError());
    }

    if (flags & SDL_WINDOW_OPENGL)
    {
        mGLContextHandle.reset(SDL_GL_CreateContext(mWindowHandle.get()));
        if (!mGLContextHandle)
        {
            throw std::runtime_error(SDL_GetError());
        }

        // wrangle GL extensions
        glewExperimental = GL_TRUE;
        GLenum glewError = glewInit();
        if (glewError != GLEW_OK)
        {
            throw std::runtime_error((const char*) glewGetErrorString(glewError));
        }
    }
}

void Window::SetPosition(int x, int y)
{
    SDL_SetWindowPosition(GetSDLHandle(), x, y);
}

int Window::GetWidth() const
{
    int w;
    SDL_GetWindowSize(GetSDLHandle(), &w, NULL);
    return w;
}

int Window::GetHeight() const
{
    int h;
    SDL_GetWindowSize(GetSDLHandle(), NULL, &h);
    return h;
}

void Window::GLSwapWindow()
{
    if (!mGLContextHandle)
    {
        throw std::runtime_error("GLSwapWindow used on non-GL window.");
    }

    SDL_GL_SwapWindow(mWindowHandle.get());
}

SDL_Window* Window::GetSDLHandle() const
{
    return mWindowHandle.get();
}

} // end namespace SDL2plus
