#include "GLplus.hpp"

#include <stdexcept>
#include <vector>

#include "SOIL2.h"

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

Shader::Shader(GLenum shaderType)
    : mHandlePtr(&mHandle, [](GLuint* handle){
        glDeleteShader(*handle);
        CheckGLErrors();
    })
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

Program::Program()
    : mHandlePtr(&mHandle, [](GLuint* handle){
        glDeleteProgram(*handle);
        CheckGLErrors();
    })
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

bool Program::TryGetUniformLocation(const GLchar* name, GLint& loc) const
{
    GLint location = glGetUniformLocation(mHandle, name);
    if (location == -1)
    {
        return false;
    }

    loc = location;
    return true;
}
GLint Program::GetUniformLocation(const GLchar* name) const
{
    GLint loc;
    if (!TryGetUniformLocation(name, loc))
    {
        throw std::runtime_error("Couldn't find uniform.");
    }
    return loc;
}

void Program::UploadUint(const GLchar* name, GLuint value) const
{
    UploadUint(GetUniformLocation(name), value);
}

void Program::UploadUint(GLint location, GLuint value) const
{
    ScopedProgramBind binder(*this);
    glUniform1i(location, value);
    CheckGLErrors();
}

void Program::UploadVec4(const GLchar* name, const GLfloat* values) const
{
    UploadVec4(GetUniformLocation(name), values);
}

void Program::UploadVec4(GLint location, const GLfloat* values) const
{
    ScopedProgramBind binder(*this);
    glUniform4fv(location, 1, values);
    CheckGLErrors();
}

void Program::UploadMatrix4(const GLchar* name, GLboolean transpose, const GLfloat* values) const
{
    UploadMatrix4(GetUniformLocation(name), transpose, values);
}

void Program::UploadMatrix4(GLint location, GLboolean transpose, const GLfloat* values) const
{
    ScopedProgramBind binder(*this);
    glUniformMatrix4fv(location, 1, transpose, values);
    CheckGLErrors();
}

GLuint Program::GetGLHandle() const
{
    return mHandle;
}

ScopedProgramBind::ScopedProgramBind(const Program& bound)
{
    glGetIntegerv(GL_CURRENT_PROGRAM, &mOldProgram);
    CheckGLErrors();

    glUseProgram(bound.GetGLHandle());
    CheckGLErrors();
}

ScopedProgramBind::~ScopedProgramBind()
{
    glUseProgram(mOldProgram);
    CheckGLErrors();
}

Buffer::Buffer(GLenum target)
    : mHandlePtr(&mHandle, [](GLuint* handle){
        glDeleteBuffers(1, handle);
        CheckGLErrors();
    })
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
    if (bound.GetTarget() == GL_ARRAY_BUFFER)
    {
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &mOldBuffer);
        CheckGLErrors();
    }
    glBindBuffer(bound.GetTarget(), bound.GetGLHandle());
    CheckGLErrors();
}

ScopedBufferBind::~ScopedBufferBind()
{
    glBindBuffer(mTarget, mOldBuffer);
    CheckGLErrors();
}

VertexArray::VertexArray()
    : mHandlePtr(&mHandle, [](GLuint* handle){
        glDeleteVertexArrays(1, handle);
        CheckGLErrors();
    })
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
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &mOldVertexArray);
    CheckGLErrors();

    glBindVertexArray(bound.GetGLHandle());
    CheckGLErrors();
}

ScopedVertexArrayBind::~ScopedVertexArrayBind()
{
    glBindVertexArray(mOldVertexArray);
    CheckGLErrors();
}

Texture2D::Texture2D()
    : mHandlePtr(&mHandle, [](GLuint* handle){
        glDeleteTextures(1, handle);
        CheckGLErrors();
    })
{
    glGenTextures(1, &mHandle);
    CheckGLErrors();

    if (!mHandle)
    {
        throw std::runtime_error("glGenTextures");
    }
}

void Texture2D::LoadImage(const char* filename, unsigned int flags)
{
    unsigned int soilFlags = 0;
    if (flags & InvertY)
    {
        soilFlags |= SOIL_FLAG_INVERT_Y;
    }

    int width, height;
    if (!SOIL_load_OGL_texture(filename,
                &width, &height, NULL,
                SOIL_LOAD_AUTO,
                mHandle,
                soilFlags))
    {
        throw std::runtime_error(SOIL_last_result());
    }

    mWidth = width;
    mHeight = height;
}

void Texture2D::CreateStorage(GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height)
{
    ScopedTextureBind binder(*this, GL_TEXTURE0);

    glTexStorage2D(GL_TEXTURE_2D, levels, internalformat, width, height);
    CheckGLErrors();

    mWidth = width;
    mHeight = height;
}

int Texture2D::GetWidth() const
{
    if (!mHandle)
    {
        throw std::runtime_error("Texture not loaded.");
    }
    return mWidth;
}

int Texture2D::GetHeight() const
{
    if (!mHandle)
    {
        throw std::runtime_error("Texture not loaded.");
    }
    return mHeight;
}

GLuint Texture2D::GetGLHandle() const
{
    return mHandle;
}

ScopedTextureBind::ScopedTextureBind(const Texture2D& bound, GLenum textureIndex)
    : mTextureIndex(textureIndex)
{
    glGetIntegerv(GL_ACTIVE_TEXTURE, &mOldTextureIndex);
    CheckGLErrors();

    glActiveTexture(mTextureIndex);
    CheckGLErrors();

    glGetIntegerv(GL_TEXTURE_BINDING_2D, &mOldTexture);
    CheckGLErrors();

    glBindTexture(GL_TEXTURE_2D, bound.GetGLHandle());
    CheckGLErrors();
}

ScopedTextureBind::~ScopedTextureBind()
{
    glBindTexture(GL_TEXTURE_2D, mOldTexture);
    CheckGLErrors();

    glActiveTexture(mOldTextureIndex);
    CheckGLErrors();
}

RenderBuffer::RenderBuffer()
    : mHandlePtr(&mHandle, [](GLuint* handle){
        glDeleteRenderbuffers(1, handle);
        CheckGLErrors();
    })
{
    glGenRenderbuffers(1, &mHandle);
    CheckGLErrors();

    if (!mHandle)
    {
        throw std::runtime_error("glGenRenderbuffers");
    }
}

void RenderBuffer::CreateStorage(GLenum internalformat, GLsizei width, GLsizei height)
{
    ScopedRenderBufferBind binder(*this);

    glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
    CheckGLErrors();
}

GLuint RenderBuffer::GetGLHandle() const
{
    return mHandle;
}

ScopedRenderBufferBind::ScopedRenderBufferBind(const RenderBuffer& bound)
{
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &mOldRenderBuffer);
    CheckGLErrors();

    glBindRenderbuffer(GL_RENDERBUFFER, bound.GetGLHandle());
    CheckGLErrors();
}

ScopedRenderBufferBind::~ScopedRenderBufferBind()
{
    glBindRenderbuffer(GL_RENDERBUFFER, mOldRenderBuffer);
    CheckGLErrors();
}

FrameBuffer::Attachment::Attachment(const std::shared_ptr<Texture2D>& texture)
    : mTextureAttachment(texture)
{
}

FrameBuffer::Attachment::Attachment(const std::shared_ptr<RenderBuffer>& renderBuffer)
    : mRenderBufferAttachment(renderBuffer)
{
}

FrameBuffer::FrameBuffer()
    : mHandlePtr(&mHandle, [](GLuint* handle){
        glDeleteFramebuffers(1, handle);
        CheckGLErrors();
    })
{
    glGenFramebuffers(1, &mHandle);
    CheckGLErrors();

    if (!mHandle)
    {
        throw std::runtime_error("glGenFramebuffers");
    }
}

void FrameBuffer::Attach(GLenum attachment, const std::shared_ptr<Texture2D>& texture)
{
    ScopedFrameBufferBind binder(*this);

    glFramebufferTexture2D(GL_FRAMEBUFFER, attachment, GL_TEXTURE_2D, texture->GetGLHandle(), 0);
    CheckGLErrors();

    mAttachments.emplace(attachment, texture);
}

void FrameBuffer::Attach(GLenum attachment, const std::shared_ptr<RenderBuffer>& renderBuffer)
{
    ScopedFrameBufferBind binder(*this);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, renderBuffer->GetGLHandle());
    CheckGLErrors();

    mAttachments.emplace(attachment, renderBuffer);
}

void FrameBuffer::Detach(GLenum attachment)
{
    ScopedFrameBufferBind binder(*this);

    glFramebufferRenderbuffer(GL_FRAMEBUFFER, attachment, GL_RENDERBUFFER, 0);
    CheckGLErrors();

    mAttachments.erase(attachment);
}

static const char* FrameBufferStatusToString(GLenum status)
{
    switch (status)
    {
    case GL_FRAMEBUFFER_COMPLETE: return "GL_FRAMEBUFFER_COMPLETE";
    case GL_FRAMEBUFFER_UNDEFINED: return "GL_FRAMEBUFFER_UNDEFINED";
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: return "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER: return "GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER";
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER: return "GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER";
    case GL_FRAMEBUFFER_UNSUPPORTED: return "GL_FRAMEBUFFER_UNSUPPORTED";
    case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE: return "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
    case GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS: return "GL_FRAMEBUFFER_INCOMPLETE_LAYER_TARGETS";
    default: return "Unknown FrameBuffer status";
    }
}

GLenum FrameBuffer::GetStatus() const
{
    ScopedFrameBufferBind binder(*this);

    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    CheckGLErrors();

    return status;
}

void FrameBuffer::ValidateStatus() const
{
    GLenum status = GetStatus();
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        throw std::runtime_error(FrameBufferStatusToString(status));
    }
}

GLuint FrameBuffer::GetGLHandle() const
{
    return mHandle;
}

ScopedFrameBufferBind::ScopedFrameBufferBind(const FrameBuffer& bound)
{
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mOldFrameBuffer);
    CheckGLErrors();

    glBindFramebuffer(GL_FRAMEBUFFER, bound.GetGLHandle());
    CheckGLErrors();
}

ScopedFrameBufferBind::ScopedFrameBufferBind(DefaultFrameBuffer)
{
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &mOldFrameBuffer);
    CheckGLErrors();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckGLErrors();
}

ScopedFrameBufferBind::~ScopedFrameBufferBind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, mOldFrameBuffer);
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
