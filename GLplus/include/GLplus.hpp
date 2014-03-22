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

    bool TryGetAttributeLocation(const GLchar* name, GLint& loc) const;
    GLint GetAttributeLocation(const GLchar* name) const;

    bool TryGetUniformLocation(const GLchar* name, GLint& loc) const;
    GLint GetUniformLocation(const GLchar* name) const;

    void UploadUint(const GLchar* name, GLuint value);
    void UploadUint(GLint location, GLuint value);

    void UploadVec4(const GLchar* name, const GLfloat* values);
    void UploadVec4(GLint location, const GLfloat* values);

    void UploadMatrix4(const GLchar* name, GLboolean transpose, const GLfloat* values);
    void UploadMatrix4(GLint location, GLboolean transpose, const GLfloat* values);

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
    GLint mOldBuffer;
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

class Texture
{
    struct TextureDeleter
    {
        void operator()(GLuint*) const;
    };

    GLuint mHandle = 0;
    std::unique_ptr<GLuint, TextureDeleter> mHandlePtr;

    int mWidth;
    int mHeight;

public:
    enum LoadFlags
    {
        InvertY
    };

    Texture();
    
    void LoadImage(const char* filename, unsigned int flags = InvertY);

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
    ScopedTextureBind(const Texture& bound, GLenum textureIndex);
    ~ScopedTextureBind();
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
