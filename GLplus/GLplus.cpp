#include "GLplus.hpp"

#include <stdexcept>
#include <vector>
#include <fstream>
#include <sstream>

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

void CheckGLErrors()
{
    GLenum firstError = glGetError();

    while (glGetError() != GL_NO_ERROR);

    if (firstError != GL_NO_ERROR)
    {
        throw std::runtime_error(StringFromGLError(firstError));
    }
}

Shader::Shader(GLenum shaderType)
    : mShaderType(shaderType)
{
    mHandle.mHandle = glCreateShader(shaderType);
    CheckGLErrors();
}

Shader::~Shader()
{
    glDeleteShader(mHandle.mHandle);
    CheckGLErrors();
}

void Shader::Compile(const GLchar* source)
{
    glShaderSource(mHandle.mHandle, 1, &source, NULL);
    CheckGLErrors();

    glCompileShader(mHandle.mHandle);
    CheckGLErrors();

    int status;
    glGetShaderiv(mHandle.mHandle, GL_COMPILE_STATUS, &status);
    CheckGLErrors();

    if (!status)
    {
        int logLength;
        glGetShaderiv(mHandle.mHandle, GL_INFO_LOG_LENGTH, &logLength);
        CheckGLErrors();

        std::vector<char> log(logLength);
        glGetShaderInfoLog(mHandle.mHandle, log.size(), NULL, log.data());
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
    return mHandle.mHandle;
}

Program::Program()
{
    mHandle.mHandle = glCreateProgram();
    CheckGLErrors();
}

Program::~Program()
{
    glDeleteProgram(mHandle.mHandle);
    CheckGLErrors();
}

void Program::Attach(const std::shared_ptr<Shader>& shader)
{
    glAttachShader(mHandle.mHandle, shader->GetGLHandle());
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
    glLinkProgram(mHandle.mHandle);
    CheckGLErrors();

    int status;
    glGetProgramiv(mHandle.mHandle, GL_LINK_STATUS, &status);
    CheckGLErrors();

    if (!status)
    {
        int logLength;
        glGetProgramiv(mHandle.mHandle, GL_INFO_LOG_LENGTH, &logLength);
        CheckGLErrors();

        std::vector<char> log(logLength);
        glGetProgramInfoLog(mHandle.mHandle, log.size(), NULL, log.data());
        CheckGLErrors();

        throw std::runtime_error(log.data());
    }
}

Program Program::FromFiles(const char* vShaderFile, const char* fShaderFile)
{
    std::ifstream vFile(vShaderFile), fFile(fShaderFile);
    if (!vFile || !fFile)
    {
        throw std::runtime_error("Couldn't open shader file");
    }

    std::stringstream vStream, fStream;
    vStream << vFile.rdbuf();
    fStream << fFile.rdbuf();

    std::shared_ptr<GLplus::Shader> vShader = std::make_shared<GLplus::Shader>(GL_VERTEX_SHADER);
    vShader->Compile(vStream.str().c_str());

    std::shared_ptr<GLplus::Shader> fShader = std::make_shared<GLplus::Shader>(GL_FRAGMENT_SHADER);
    fShader->Compile(fStream.str().c_str());

    // attach & link
    Program program;
    program.Attach(vShader);
    program.Attach(fShader);
    program.Link();

    return program;
}

bool Program::TryGetAttributeLocation(const GLchar* name, GLint& loc) const
{
    GLint location = glGetAttribLocation(mHandle.mHandle, name);
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
    GLint location = glGetUniformLocation(mHandle.mHandle, name);
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
    return mHandle.mHandle;
}

ScopedProgramBind::ScopedProgramBind(const Program& bound)
{
    GLint currentProgram;
    glGetIntegerv(GL_CURRENT_PROGRAM, &currentProgram);
    CheckGLErrors();

    mOldProgram.mHandle = currentProgram;

    glUseProgram(bound.GetGLHandle());
    CheckGLErrors();
}

ScopedProgramBind::~ScopedProgramBind()
{
    glUseProgram(mOldProgram.mHandle);
    CheckGLErrors();
}

Buffer::Buffer(GLenum target)
    : mTarget(target)
{
    glGenBuffers(1, &mHandle.mHandle);
    CheckGLErrors();
}

Buffer::~Buffer()
{
    glDeleteBuffers(1, &mHandle.mHandle);
    CheckGLErrors();
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
    return mHandle.mHandle;
}

ScopedBufferBind::ScopedBufferBind(const Buffer& bound)
    : mTarget(bound.GetTarget())
{
    if (bound.GetTarget() == GL_ARRAY_BUFFER)
    {
        GLint oldBuffer;
        glGetIntegerv(GL_ARRAY_BUFFER_BINDING, &oldBuffer);
        CheckGLErrors();

        mOldBuffer.mHandle = oldBuffer;
    }
    glBindBuffer(bound.GetTarget(), bound.GetGLHandle());
    CheckGLErrors();
}

ScopedBufferBind::~ScopedBufferBind()
{
    glBindBuffer(mTarget, mOldBuffer.mHandle);
    CheckGLErrors();
}

VertexArray::VertexArray()
{
    glGenVertexArrays(1, &mHandle.mHandle);
    CheckGLErrors();
}

VertexArray::~VertexArray()
{
    glDeleteVertexArrays(1, &mHandle.mHandle);
    CheckGLErrors();
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
    return mHandle.mHandle;
}

ScopedVertexArrayBind::ScopedVertexArrayBind(const VertexArray& bound)
{
    GLint oldArray;
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &oldArray);
    CheckGLErrors();

    mOldVertexArray.mHandle = oldArray;

    glBindVertexArray(bound.GetGLHandle());
    CheckGLErrors();
}

ScopedVertexArrayBind::~ScopedVertexArrayBind()
{
    glBindVertexArray(mOldVertexArray.mHandle);
    CheckGLErrors();
}

Texture2D::Texture2D()
{
    glGenTextures(1, &mHandle.mHandle);
    CheckGLErrors();
}

Texture2D::~Texture2D()
{
    glDeleteTextures(1, &mHandle.mHandle);
    CheckGLErrors();
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
                mHandle.mHandle,
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
    if (!mHandle.mHandle)
    {
        throw std::runtime_error("Texture not loaded.");
    }
    return mWidth;
}

int Texture2D::GetHeight() const
{
    if (!mHandle.mHandle)
    {
        throw std::runtime_error("Texture not loaded.");
    }
    return mHeight;
}

GLuint Texture2D::GetGLHandle() const
{
    return mHandle.mHandle;
}

ScopedTextureBind::ScopedTextureBind(const Texture2D& bound, GLenum textureIndex)
    : mTextureIndex(textureIndex)
{
    glGetIntegerv(GL_ACTIVE_TEXTURE, &mOldTextureIndex);
    CheckGLErrors();

    glActiveTexture(mTextureIndex);
    CheckGLErrors();

    GLint oldTexture;
    glGetIntegerv(GL_TEXTURE_BINDING_2D, &oldTexture);
    CheckGLErrors();

    mOldTexture.mHandle = oldTexture;

    glBindTexture(GL_TEXTURE_2D, bound.GetGLHandle());
    CheckGLErrors();
}

ScopedTextureBind::~ScopedTextureBind()
{
    glBindTexture(GL_TEXTURE_2D, mOldTexture.mHandle);
    CheckGLErrors();

    glActiveTexture(mOldTextureIndex);
    CheckGLErrors();
}

RenderBuffer::RenderBuffer()
{
    glGenRenderbuffers(1, &mHandle.mHandle);
    CheckGLErrors();
}

RenderBuffer::~RenderBuffer()
{
    glDeleteRenderbuffers(1, &mHandle.mHandle);
    CheckGLErrors();
}

void RenderBuffer::CreateStorage(GLenum internalformat, GLsizei width, GLsizei height)
{
    ScopedRenderBufferBind binder(*this);

    glRenderbufferStorage(GL_RENDERBUFFER, internalformat, width, height);
    CheckGLErrors();
}

GLuint RenderBuffer::GetGLHandle() const
{
    return mHandle.mHandle;
}

ScopedRenderBufferBind::ScopedRenderBufferBind(const RenderBuffer& bound)
{
    GLint oldRenderbuffer;
    glGetIntegerv(GL_RENDERBUFFER_BINDING, &oldRenderbuffer);
    CheckGLErrors();

    mOldRenderBuffer.mHandle = oldRenderbuffer;

    glBindRenderbuffer(GL_RENDERBUFFER, bound.GetGLHandle());
    CheckGLErrors();
}

ScopedRenderBufferBind::~ScopedRenderBufferBind()
{
    glBindRenderbuffer(GL_RENDERBUFFER, mOldRenderBuffer.mHandle);
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
{
    glGenFramebuffers(1, &mHandle.mHandle);
    CheckGLErrors();
}

FrameBuffer::~FrameBuffer()
{
    glDeleteFramebuffers(1, &mHandle.mHandle);
    CheckGLErrors();
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
    return mHandle.mHandle;
}

ScopedFrameBufferBind::ScopedFrameBufferBind(const FrameBuffer& bound)
{
    GLint oldFramebuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFramebuffer);
    CheckGLErrors();

    mOldFrameBuffer.mHandle = oldFramebuffer;

    glBindFramebuffer(GL_FRAMEBUFFER, bound.GetGLHandle());
    CheckGLErrors();
}

ScopedFrameBufferBind::ScopedFrameBufferBind(DefaultFrameBuffer)
{
    GLint oldFramebuffer;
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &oldFramebuffer);
    CheckGLErrors();

    mOldFrameBuffer.mHandle = oldFramebuffer;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    CheckGLErrors();
}

ScopedFrameBufferBind::~ScopedFrameBufferBind()
{
    CheckGLErrors();
    glBindFramebuffer(GL_FRAMEBUFFER, mOldFrameBuffer.mHandle);
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
    CheckGLErrors();
}

} // end namespace GLplus
