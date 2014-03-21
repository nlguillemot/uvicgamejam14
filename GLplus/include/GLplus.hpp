#ifndef GLPLUS_H
#define GLPLUS_H

#include <GL/glew.h>

#include <memory>
#include <unordered_map>

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

    GLenum mShaderType;

public:
    Shader(GLenum shaderType);

    void Compile(const GLchar* source);

    GLenum GetShaderType() const;

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

    std::shared_ptr<Shader> mFragmentShader;
    std::shared_ptr<Shader> mVertexShader;

public:
    Program();

    void Attach(const std::shared_ptr<Shader>& shader);
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

class Buffer
{
    struct BufferDeleter
    {
        void operator()(GLuint* handle) const;
    };

    GLuint mHandle = 0;
    std::unique_ptr<GLuint, BufferDeleter> mHandlePtr;

    GLenum mTarget;

public:
    Buffer(GLenum target);

    void Upload(GLsizeiptr size, const GLvoid* data, GLenum usage);

    GLenum GetTarget() const;

    GLuint GetGLHandle() const;
};

class ScopedBufferBind
{
    GLenum mTarget;

public:
    ScopedBufferBind(const Buffer& bound);
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

    std::unordered_map<GLuint, std::shared_ptr<Buffer> > mVertexBuffers;
    std::shared_ptr<Buffer> mIndexBuffer;

public:
    VertexArray();

    void SetAttribute(
            GLuint index,
            const std::shared_ptr<Buffer>& buffer,
            GLint size,
            GLenum type,
            GLboolean normalized,
            GLsizei stride,
            GLsizei offset);

    void SetIndexBuffer(const std::shared_ptr<Buffer>& buffer);

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
