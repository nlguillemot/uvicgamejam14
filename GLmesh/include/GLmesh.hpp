#ifndef GLMESH_H
#define GLMESH_H

#include <GLplus.hpp>

namespace tinyobj
{
    struct shape_t;
} // end namespace tinyobj

namespace GLmesh
{

class StaticMesh
{
    std::shared_ptr<GLplus::Buffer> mPositions;
    std::shared_ptr<GLplus::Buffer> mTexcoords;
    std::shared_ptr<GLplus::Buffer> mNormals;
    std::shared_ptr<GLplus::Buffer> mIndices;

    size_t mVertexCount = 0;

    std::shared_ptr<GLplus::Texture2D> mDiffuseTexture;

public:
    void LoadShape(const tinyobj::shape_t& shape);

    void Render(const GLplus::Program& program) const;
};

} // end namespace GLmesh

#endif // GLMESH_H
