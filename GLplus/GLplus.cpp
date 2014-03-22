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

Shader::Shader(GLenum shaderType)
    : mHandlePtr(&mHandle)
    , mShaderType(shaderType)
{
    mHandle = glCreateShader(shaderType);
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

GLenum Shader::GetShaderType() const
{
    return mShaderType;
}

GLuint Shader::GetGLHandle() const
{
    return mHandle;
}

void Program::ProgramDeleter::operator()(GLuint* handle) const
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

void Program::Attach(const std::shared_ptr<Shader>& shader)
{
    glAttachShader(mHandle, shader->GetGLHandle());
    CheckGLErrors();

    switch (shader->GetShaderType())
    {
    case GL_FRAGMENT_SHADER:
        mFragmentShader = shader;
        break;
    case GL_VERTEX_SHADER:
        mVertexShader = shader;
        break;
    default:
        throw std::runtime_error("Unknown shader type.");
    }
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

bool Program::TryGetAttributeLocation(const GLchar* name, GLint& loc) const
{
    GLint location = glGetAttribLocation(mHandle, name);
    CheckGLErrors();
    if (location == -1)
    {
        return false;
    }

    loc = location;
    return true;
}

GLint Program::GetAttributeLocation(const GLchar* name) const
{
    GLint loc;
    if (!TryGetAttributeLocation(name, loc))
    {
        throw std::runtime_error("Couldn't find attribute.");
    }
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

void Buffer::BufferDeleter::operator()(GLuint* handle) const
{
    glDeleteBuffers(1, handle);
    CheckGLErrors();
}

Buffer::Buffer(GLenum target)
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

void Buffer::Upload(GLsizeiptr size, const GLvoid* data, GLenum usage)
{
    ScopedBufferBind binder(*this);

    glBufferData(mTarget, size, data, usage);
    CheckGLErrors();
}

GLenum Buffer::GetTarget() const
{
    return mTarget;
}

GLuint Buffer::GetGLHandle() const
{
    return mHandle;
}

ScopedBufferBind::ScopedBufferBind(const Buffer& bound)
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
        const std::shared_ptr<Buffer>& buffer,
        GLint size,
        GLenum type,
        GLboolean normalized,
        GLsizei stride,
        GLsizei offset)
{
    if (buffer->GetTarget() != GL_ARRAY_BUFFER)
    {
        throw std::logic_error("Only GL_ARRAY_BUFFERs can be used as attributes.");
    }

    ScopedVertexArrayBind binder(*this);

    glEnableVertexAttribArray(index);
    CheckGLErrors();

    {
        ScopedBufferBind bufferBind(*buffer);

        glVertexAttribPointer(index, size, type, normalized, stride, (const GLvoid*) offset);
        CheckGLErrors();

        mVertexBuffers[index] = buffer;
    }
}

void VertexArray::SetIndexBuffer(const std::shared_ptr<Buffer>& buffer, GLenum type)
{
    if (buffer->GetTarget() != GL_ELEMENT_ARRAY_BUFFER)
    {
        throw std::logic_error("Only GL_ELEMENT_ARRAY_BUFFERs can be used as index buffers.");
    }

    ScopedVertexArrayBind binder(*this);

    // spookiest, most unobviously documented thing about the GL spec I found so far.
    glBindBuffer(buffer->GetTarget(), buffer->GetGLHandle());
    CheckGLErrors();

    mIndexBuffer = buffer;
    mIndexType = type;
}

GLenum VertexArray::GetIndexType() const
{
    if (!mIndexType)
    {
        throw std::runtime_error("VertexArray has no index type.");
    }
    return mIndexType;
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

void DrawElements(const Program& program, const VertexArray& model,
                  GLenum mode, GLint first, GLsizei count)
{
    ScopedProgramBind programBind(program);
    ScopedVertexArrayBind modelBind(model);

    glDrawElements(mode, count, model.GetIndexType(),
                   (const GLvoid*) (SizeFromGLType(model.GetIndexType()) * first));
}

} // end namespace GLplus
