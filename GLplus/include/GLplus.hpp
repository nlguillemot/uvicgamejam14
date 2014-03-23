#ifndef GLPLUS_H
#define GLPLUS_H

#include <GL/glew.h>

#include <memory>
#include <unordered_map>

namespace GLplus
{

void CheckGLErrors();

struct ObjectHandle
{
    GLuint mHandle;
    ObjectHandle(){ mHandle = 0; }
    ObjectHandle(const ObjectHandle& other) = delete;
    ObjectHandle& operator=(const ObjectHandle& other) = delete;
    ObjectHandle(ObjectHandle&& other){ std::swap(mHandle, other.mHandle); }
    ObjectHandle& operator=(ObjectHandle&& other){ std::swap(mHandle, other.mHandle); }
};

class Shader
{
    ObjectHandle mHandle;
    GLenum mShaderType;

public:
    Shader(GLenum shaderType);
    Shader(const Shader&) = delete;
    Shader& operator=(const Shader&) = delete;
    Shader(Shader&&) = default;
    Shader& operator=(Shader&&) = default;
    ~Shader();

    void Compile(const GLchar* source);

    GLenum GetShaderType() const;

    GLuint GetGLHandle() const;
};

class Program
{
    ObjectHandle mHandle;
    std::shared_ptr<Shader> mFragmentShader;
    std::shared_ptr<Shader> mVertexShader;

public:
    static Program FromFiles(const char* vShaderFile, const char* fShaderFile);

    Program();
    Program(const Program&) = delete;
    Program& operator=(const Program&) = delete;
    Program(Program&&) = default;
    Program& operator=(Program&&) = default;
    ~Program();

    void Attach(const std::shared_ptr<Shader>& shader);
    void Link();

    bool TryGetAttributeLocation(const GLchar* name, GLint& loc) const;
    GLint GetAttributeLocation(const GLchar* name) const;

    bool TryGetUniformLocation(const GLchar* name, GLint& loc) const;
    GLint GetUniformLocation(const GLchar* name) const;

    void UploadUint(const GLchar* name, GLuint value) const;
    void UploadUint(GLint location, GLuint value) const;

    void UploadVec4(const GLchar* name, const GLfloat* const values) const;
    void UploadVec4(GLint location, const GLfloat* values) const;

    void UploadMatrix4(const GLchar* name, GLboolean transpose, const GLfloat* values) const;
    void UploadMatrix4(GLint location, GLboolean transpose, const GLfloat* values) const;

    GLuint GetGLHandle() const;
};

class ScopedProgramBind
{
    ObjectHandle mOldProgram;

public:
    ScopedProgramBind(const Program& bound);
    ~ScopedProgramBind();
};

class Buffer
{
    ObjectHandle mHandle;
    GLenum mTarget;

public:
    Buffer(GLenum target);
    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;
    Buffer(Buffer&&) = default;
    Buffer& operator=(Buffer&&) = default;
    ~Buffer();

    void Upload(GLsizeiptr size, const GLvoid* data, GLenum usage);

    GLenum GetTarget() const;

    GLuint GetGLHandle() const;
};

class ScopedBufferBind
{
    ObjectHandle mOldBuffer;
    GLenum mTarget;

public:
    ScopedBufferBind(const Buffer& bound);
    ~ScopedBufferBind();
};

class VertexArray
{
    ObjectHandle mHandle;
    std::unordered_map<GLuint, std::shared_ptr<Buffer> > mVertexBuffers;
    std::shared_ptr<Buffer> mIndexBuffer;
    GLenum mIndexType = 0;

public:
    VertexArray();
    VertexArray(const VertexArray&) = delete;
    VertexArray& operator=(const VertexArray&) = delete;
    VertexArray(VertexArray&&) = default;
    VertexArray& operator=(VertexArray&&) = default;
    ~VertexArray();

    void SetAttribute(
            GLuint index,
            const std::shared_ptr<Buffer>& buffer,
            GLint size,
            GLenum type,
            GLboolean normalized,
            GLsizei stride,
            GLsizei offset);

    void SetIndexBuffer(
            const std::shared_ptr<Buffer>& buffer,
            GLenum type);

    GLenum GetIndexType() const;

    GLuint GetGLHandle() const;
};

class ScopedVertexArrayBind
{
    ObjectHandle mOldVertexArray;

public:
    ScopedVertexArrayBind(const VertexArray& bound);
    ~ScopedVertexArrayBind();
};

class Texture2D
{
    ObjectHandle mHandle;
    int mWidth;
    int mHeight;

public:
    enum LoadFlags
    {
        NoFlags = 0,
        InvertY
    };

    Texture2D();
    Texture2D(const Texture2D&) = delete;
    Texture2D& operator=(const Texture2D&) = delete;
    Texture2D(Texture2D&&) = default;
    Texture2D& operator=(Texture2D&&) = default;
    ~Texture2D();

    void LoadImage(const char* filename, unsigned int flags);
    void CreateStorage(GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);

    int GetWidth() const;
    int GetHeight() const;

    GLuint GetGLHandle() const;
};

class ScopedTextureBind
{
    ObjectHandle mOldTexture;
    GLint mOldTextureIndex;
    GLenum mTextureIndex;

public:
    ScopedTextureBind(const Texture2D& bound, GLenum textureIndex);
    ~ScopedTextureBind();
};

class RenderBuffer
{
    ObjectHandle mHandle;

public:
    RenderBuffer();
    RenderBuffer(const RenderBuffer&) = delete;
    RenderBuffer& operator=(const RenderBuffer&) = delete;
    RenderBuffer(RenderBuffer&&) = default;
    RenderBuffer& operator=(RenderBuffer&&) = default;
    ~RenderBuffer();

    void CreateStorage(GLenum internalformat, GLsizei width, GLsizei height);

    GLuint GetGLHandle() const;
};

class ScopedRenderBufferBind
{
    ObjectHandle mOldRenderBuffer;

public:
    ScopedRenderBufferBind(const RenderBuffer& bound);
    ~ScopedRenderBufferBind();
};

class FrameBuffer
{
    ObjectHandle mHandle;

    struct Attachment
    {
        Attachment(const std::shared_ptr<Texture2D>&);
        Attachment(const std::shared_ptr<RenderBuffer>&);
        std::shared_ptr<Texture2D> mTextureAttachment;
        std::shared_ptr<RenderBuffer> mRenderBufferAttachment;
    };

    std::unordered_map<GLenum, Attachment> mAttachments;

public:
    FrameBuffer();
    FrameBuffer(const FrameBuffer&) = delete;
    FrameBuffer& operator=(const FrameBuffer&) = delete;
    FrameBuffer(FrameBuffer&&) = default;
    FrameBuffer& operator=(FrameBuffer&&) = default;
    ~FrameBuffer();

    void Attach(GLenum attachment, const std::shared_ptr<Texture2D>& texture);
    void Attach(GLenum attachment, const std::shared_ptr<RenderBuffer>& renderBuffer);

    void Detach(GLenum attachment);

    GLenum GetStatus() const;
    void ValidateStatus() const;

    GLuint GetGLHandle() const;
};

class DefaultFrameBuffer { };

// TODO: Make it possible to choose between the DRAW/READ framebuffer
class ScopedFrameBufferBind
{
    ObjectHandle mOldFrameBuffer;

public:
    ScopedFrameBufferBind(const FrameBuffer& bound);
    ScopedFrameBufferBind(DefaultFrameBuffer);
    ~ScopedFrameBufferBind();
};

constexpr size_t SizeFromGLType(GLenum type)
{
    return type == GL_UNSIGNED_INT   ? sizeof(GLuint)   :
           type == GL_INT            ? sizeof(GLint)    :
           type == GL_UNSIGNED_SHORT ? sizeof(GLushort) :
           type == GL_SHORT          ? sizeof(GLshort)  :
           type == GL_UNSIGNED_BYTE  ? sizeof(GLubyte)  :
           type == GL_BYTE           ? sizeof(GLbyte)   :
           throw "Unimplemented Type";
}

void DrawArrays(const Program& program, const VertexArray& model,
                GLenum mode, GLint first, GLsizei count);

void DrawElements(const Program& program, const VertexArray& model,
                GLenum mode, GLint first, GLsizei count);

} // end namespace GLplus

#endif // GLPLUS_H
