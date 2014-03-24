#ifndef SDL2PLUS_H
#define SDL2PLUS_H

#include <SDL.h>

#include <memory>
#include <type_traits>

namespace SDL2plus
{

class LibSDL
{
public:
    LibSDL(Uint32 flags);
    ~LibSDL();

    void SetGLAttribute(SDL_GLattr attr, int value);
};

class Window
{
    // TODO: fix this totally broken raii crap
    struct WindowDeleter
    {
        void operator()(SDL_Window*) const;
    };

    std::unique_ptr<SDL_Window, WindowDeleter> mWindowHandle;

    struct GLContextDeleter
    {
        void operator()(SDL_GLContext context) const;
    };

    std::unique_ptr<std::remove_pointer<SDL_GLContext>::type, GLContextDeleter> mGLContextHandle;

public:
    Window(int width, int height, const char* title, Uint32 flags = 0);

    void SetPosition(int x, int y);

    int GetWidth() const;
    int GetHeight() const;

    void GLSwapWindow();

    SDL_Window* GetSDLHandle() const;
};

} // end namespace SDL2plus

#endif // SDL2PLUS_H
