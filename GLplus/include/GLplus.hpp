#ifndef GLPLUS_H
#define GLPLUS_H

#include <GL/glew.h>

#include <memory>
#include <unordered_map>

namespace GLplus
{

class Shader
{
    GLuint mHandle = 0;
    std::unique_ptr<GLuint, void(*)(GLuint*)> mHandlePtr;

    GLenum mShaderType;

public:
    Shader(GLenum shaderType);

    void Compile(const GLchar* source);

    GLenum GetShaderType() const;

    GLuint GetGLHandle() const;
};

class Program
{
    GLuint mHandle = 0;
    std::unique_ptr<GLuint, void(*)(GLuint*)> mHandlePtr;

    std::shared_ptr<Shader> mFragmentShader;
    std::shared_ptr<Shader> mVertexShader;

public:
    static Program FromFiles(const char* vShaderFile, const char* fShaderFile);

    Program();

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
    GLint mOldProgram;

public:
    ScopedProgramBind(const Program& bound);
    ~ScopedProgramBind();
};

class Buffer
{
    GLuint mHandle = 0;
    std::unique_ptr<GLuint, void(*)(GLuint*)> mHandlePtr;

    GLenum mTarget;

public:
    Buffer(GLenum target);

    void Upload(GLsizeiptr size, const GLvoid* data, GLenum usage);

    GLenum GetTarget() const;

    GLuint GetGLHandle() const;
};

class ScopedBufferBind
{
    GLint mOldBuffer;
    GLenum mTarget;

public:
    ScopedBufferBind(const Buffer& bound);
    ~ScopedBufferBind();
};

class VertexArray
{
    GLuint mHandle = 0;
    std::unique_ptr<GLuint, void(*)(GLuint*)> mHandlePtr;

    std::unordered_map<GLuint, std::shared_ptr<Buffer> > mVertexBuffers;
    std::shared_ptr<Buffer> mIndexBuffer;
    GLenum mIndexType = 0;

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

    void SetIndexBuffer(
            const std::shared_ptr<Buffer>& buffer,
            GLenum type);

    GLenum GetIndexType() const;

    GLuint GetGLHandle() const;
};

class ScopedVertexArrayBind
{
    GLint mOldVertexArray;

public:
    ScopedVertexArrayBind(const VertexArray& bound);
    ~ScopedVertexArrayBind();
};

class Texture2D
{
    GLuint mHandle = 0;
    std::unique_ptr<GLuint, void(*)(GLuint*)> mHandlePtr;

    int mWidth;
    int mHeight;

public:
    enum LoadFlags
    {
        NoFlags = 0,
        InvertY
    };

    Texture2D();

    void LoadImage(const char* filename, unsigned int flags);
    void CreateStorage(GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);

    int GetWidth() const;
    int GetHeight() const;

    GLuint GetGLHandle() const;
};

class ScopedTextureBind
{
    GLint mOldTexture;
    GLint mOldTextureIndex;
    GLenum mTextureIndex;

public:
    ScopedTextureBind(const Texture2D& bound, GLenum textureIndex);
    ~ScopedTextureBind();
};

class RenderBuffer
{
    GLuint mHandle = 0;
    std::unique_ptr<GLuint, void(*)(GLuint*)> mHandlePtr;

public:
    RenderBuffer();

    void CreateStorage(GLenum internalformat, GLsizei width, GLsizei height);

    GLuint GetGLHandle() const;
};

class ScopedRenderBufferBind
{
    GLint mOldRenderBuffer;

public:
    ScopedRenderBufferBind(const RenderBuffer& bound);
    ~ScopedRenderBufferBind();
};

class FrameBuffer
{
    GLuint mHandle;
    std::unique_ptr<GLuint, void(*)(GLuint*)> mHandlePtr;

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
    GLint mOldFrameBuffer;

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
