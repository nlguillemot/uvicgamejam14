#include "GLplus.hpp"

#include <stdexcept>
#include <vector>

namespace GLplus
{

static const char* StringFromGLError(GLenum err)
{
    switch (err)
    {
    case GL_NO_ERROR:          return "GL_NO_ERROR";
    case GL_INVALID_ENUM:      return "GL_INVALID_ENUM";
    case GL_INVALID_VALUE:     return "GL_INVALID_VALUE";
    case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
    case GL_STACK_OVERFLOW:    return "GL_STACK_OVERFLOW";
    case GL_STACK_UNDERFLOW:   return "GL_STACK_UNDERFLOW";
    case GL_OUT_OF_MEMORY:     return "GL_OUT_OF_MEMORY";
    default:                   return "Unknown GL error";
    }
}

static void CheckGLErrors()
{
    GLenum firstError = glGetError();

    while (glGetError() != GL_NO_ERROR);

    if (firstError != GL_NO_ERROR)
    {
        throw std::runtime_error(StringFromGLError(firstError));
    }
}


void Shader::ShaderDeleter::operator ()(GLuint* handle) const
{
    glDeleteShader(*handle);
    CheckGLErrors();
}

Shader::Shader(GLenum shadertype)
    : mHandlePtr(&mHandle)
{
    mHandle = glCreateShader(shadertype);
    CheckGLErrors();

    if (!mHandle)
    {
        throw std::runtime_error("glCreateShader");
    }
}

void Shader::Compile(const GLchar* source)
{
    glShaderSource(mHandle, 1, &source, NULL);
    CheckGLErrors();

    glCompileShader(mHandle);
    CheckGLErrors();

    int status;
    glGetShaderiv(mHandle, GL_COMPILE_STATUS, &status);
    CheckGLErrors();

    if (!status)
    {
        int logLength;
        glGetShaderiv(mHandle, GL_INFO_LOG_LENGTH, &logLength);
        CheckGLErrors();

        std::vector<char> log(logLength);
        glGetShaderInfoLog(mHandle, log.size(), NULL, log.data());
        CheckGLErrors();

        throw std::runtime_error(log.data());
    }
}

GLuint Shader::GetGLHandle() const
{
    return mHandle;
}

void Program::ProgramDeleter::operator ()(GLuint* handle) const
{
    glDeleteProgram(*handle);
    CheckGLErrors();
}

Program::Program()
    : mHandlePtr(&mHandle)
{
    mHandle = glCreateProgram();
    CheckGLErrors();

    if (!mHandle)
    {
        throw std::runtime_error("glCreateProgram");
    }
}

void Program::Attach(Shader& shader)
{
    glAttachShader(mHandle, shader.GetGLHandle());
    CheckGLErrors();
}

void Program::Link()
{
    glLinkProgram(mHandle);
    CheckGLErrors();

    int status;
    glGetProgramiv(mHandle, GL_LINK_STATUS, &status);
    CheckGLErrors();

    if (!status)
    {
        int logLength;
        glGetProgramiv(mHandle, GL_INFO_LOG_LENGTH, &logLength);
        CheckGLErrors();

        std::vector<char> log(logLength);
        glGetProgramInfoLog(mHandle, log.size(), NULL, log.data());
        CheckGLErrors();

        throw std::runtime_error(log.data());
    }
}

GLint Program::GetAttributeLocation(const GLchar* name) const
{
    GLint loc = glGetAttribLocation(mHandle, name);
    CheckGLErrors();
    return loc;
}

GLuint Program::GetGLHandle() const
{
    return mHandle;
}

ScopedProgramBind::ScopedProgramBind(const Program& bound)
{
    glUseProgram(bound.GetGLHandle());
    CheckGLErrors();
}

ScopedProgramBind::~ScopedProgramBind()
{
    glUseProgram(0);
    CheckGLErrors();
}

void VertexBuffer::VertexBufferDeleter::operator()(GLuint* handle) const
{
    glDeleteBuffers(1, handle);
    CheckGLErrors();
}

VertexBuffer::VertexBuffer(GLenum target)
    : mHandlePtr(&mHandle)
    , mTarget(target)
{
    glGenBuffers(1, &mHandle);
    CheckGLErrors();

    if (!mHandle)
    {
        throw std::runtime_error("glGenBuffers");
    }
}

void VertexBuffer::Upload(GLsizeiptr size, const GLvoid* data, GLenum usage)
{
    ScopedBufferBind binder(*this);

    glBufferData(mTarget, size, data, usage);
    CheckGLErrors();
}

GLenum VertexBuffer::GetTarget() const
{
    return mTarget;
}

GLuint VertexBuffer::GetGLHandle() const
{
    return mHandle;
}

ScopedBufferBind::ScopedBufferBind(const VertexBuffer& bound)
    : mTarget(bound.GetTarget())
{
    glBindBuffer(bound.GetTarget(), bound.GetGLHandle());
    CheckGLErrors();
}

ScopedBufferBind::~ScopedBufferBind()
{
    glBindBuffer(mTarget, 0);
    CheckGLErrors();
}

void VertexArray::VertexArrayDeleter::operator()(GLuint* handle) const
{
    glDeleteVertexArrays(1, handle);
    CheckGLErrors();
}

VertexArray::VertexArray()
    : mHandlePtr(&mHandle)
{
    glGenVertexArrays(1, &mHandle);
    CheckGLErrors();

    if (!mHandle)
    {
        throw std::runtime_error("glGenVertexArrays");
    }
}

void VertexArray::SetAttribute(
        GLuint index,
        const VertexBuffer& buffer,
        GLint size,
        GLenum type,
        GLboolean normalized,
        GLsizei stride,
        GLsizei offset)
{
    if (buffer.GetTarget() != GL_ARRAY_BUFFER)
    {
        throw std::logic_error("Only GL_ARRAY_BUFFERs can be used as attributes.");
    }

    ScopedVertexArrayBind binder(*this);

    glEnableVertexAttribArray(index);
    CheckGLErrors();

    {
        ScopedBufferBind bufferBind(buffer);

        glVertexAttribPointer(index, size, type, normalized, stride, (const GLvoid*) offset);
        CheckGLErrors();
    }
}

void VertexArray::SetIndexBuffer(const VertexBuffer& buffer)
{
    if (buffer.GetTarget() != GL_ELEMENT_ARRAY_BUFFER)
    {
        throw std::logic_error("Only GL_ELEMENT_ARRAY_BUFFERs can be used as index buffers.");
    }

    ScopedVertexArrayBind binder(*this);

    // spookiest, most unobviously documented thing about the GL spec I found so far.
    glBindBuffer(buffer.GetTarget(), buffer.GetGLHandle());
    CheckGLErrors();
}

GLuint VertexArray::GetGLHandle() const
{
    return mHandle;
}

ScopedVertexArrayBind::ScopedVertexArrayBind(const VertexArray& bound)
{
    glBindVertexArray(bound.GetGLHandle());
    CheckGLErrors();
}

ScopedVertexArrayBind::~ScopedVertexArrayBind()
{
    glBindVertexArray(0);
    CheckGLErrors();
}

void DrawArrays(const Program &program, const VertexArray &model,
                GLenum mode, GLint first, GLsizei count)
{
    ScopedProgramBind programBind(program);
    ScopedVertexArrayBind modelBind(model);

    glDrawArrays(mode, first, count);
    CheckGLErrors();
}

} // end namespace GLplus
