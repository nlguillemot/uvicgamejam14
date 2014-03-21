#ifndef GLPLUS_H
#define GLPLUS_H

#include <GL/glew.h>

#include <memory>

namespace GLplus
{

class Shader
{
    struct ShaderDeleter
    {
        void operator()(GLuint* handle) const;
    };

    GLuint mHandle = 0;
    std::unique_ptr<GLuint, ShaderDeleter> mHandlePtr;

public:
    Shader(GLenum shadertype);

    void Compile(const GLchar* source);

    GLuint GetGLHandle() const;
};

class Program
{
    struct ProgramDeleter
    {
        void operator()(GLuint* handle) const;
    };

    GLuint mHandle = 0;
    std::unique_ptr<GLuint, ProgramDeleter> mHandlePtr;

public:
    Program();

    void Attach(Shader& shader);
    void Link();

    GLint GetAttributeLocation(const GLchar* name) const;

    GLuint GetGLHandle() const;
};

class ScopedProgramBind
{
public:
    ScopedProgramBind(const Program& bound);
    ~ScopedProgramBind();
};

class VertexBuffer
{
    struct VertexBufferDeleter
    {
        void operator()(GLuint* handle) const;
    };

    GLuint mHandle = 0;
    std::unique_ptr<GLuint, VertexBufferDeleter> mHandlePtr;

    GLenum mTarget;

public:
    VertexBuffer(GLenum target);

    void Upload(GLsizeiptr size, const GLvoid* data, GLenum usage);

    GLenum GetTarget() const;

    GLuint GetGLHandle() const;
};

class ScopedBufferBind
{
    GLenum mTarget;

public:
    ScopedBufferBind(const VertexBuffer& bound);
    ~ScopedBufferBind();
};

class VertexArray
{
    struct VertexArrayDeleter
    {
        void operator()(GLuint* handle) const;
    };

    GLuint mHandle = 0;
    std::unique_ptr<GLuint, VertexArrayDeleter> mHandlePtr;

public:
    VertexArray();

    void SetAttribute(
            GLuint index,
            const VertexBuffer& buffer,
            GLint size,
            GLenum type,
            GLboolean normalized,
            GLsizei stride,
            GLsizei offset);

    void SetIndexBuffer(const VertexBuffer& buffer);

    GLuint GetGLHandle() const;
};

class ScopedVertexArrayBind
{
public:
    ScopedVertexArrayBind(const VertexArray& bound);
    ~ScopedVertexArrayBind();
};

void DrawArrays(const Program& program, const VertexArray& model,
                GLenum mode, GLint first, GLsizei count);

} // end namespace GLplus

#endif // GLPLUS_H
