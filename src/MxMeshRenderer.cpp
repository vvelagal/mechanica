/*
 * MxMeshRenderer.cpp
 *
 *  Created on: Jul 7, 2017
 *      Author: andy
 */

#include <iostream>
#include <MxMeshRenderer.h>
#include <MxMeshShaderProgram.h>

typedef MxMeshShaderProgram ShaderProgram;


MxMeshRenderer::MxMeshRenderer(Flags flags) :
        shader{new MxMeshShaderProgram(flags)},
        mesh{nullptr}
{
}

MxMeshRenderer& MxMeshRenderer::setViewportSize(const Magnum::Vector2& size) {
    shader->setViewportSize(size);
    return *this;
}

MxMeshRenderer& MxMeshRenderer::setColor(const Magnum::Color4& color) {
    shader->setColor(color);
    return *this;
}

MxMeshRenderer& MxMeshRenderer::setWireframeColor(const Magnum::Color4& color) {
    shader->setWireframeColor(color);
    return *this;
}

MxMeshRenderer& MxMeshRenderer::setWireframeWidth(float width) {
    shader->setWireframeWidth(width);
    return *this;
}

MxMeshRenderer& MxMeshRenderer::setSmoothness(float smoothness) {
    shader->setSmoothness(smoothness);
    return *this;
}

void dumpVertex(void* vertex, uint vertexCount) {

    Vector3 *p = (Vector3*)vertex;

    std::cout << "vertices: " << std::endl;
    std::cout << "---------" << std::endl;
    for (int i = 0; i < vertexCount; ++i) {
        std::cout << "{" << p[i][0] << "," << p[i][1] << "," << p[i][2] << "}" << std::endl;
        if ((i+1) % 3 == 0) {
            std::cout << "---------" << std::endl;

        }
    }
}

void dumpIndex(void* ind, uint indCount) {
    uint faceCnt = indCount / 3;
    uint *pint = (uint*)ind;

    std::cout << "indices: " << std::endl;
    for(int i = 0; i < faceCnt; ++i) {
        std::cout << "{" << pint[3*i] << "," << pint[3*i+1] << "," << pint[3*i+2] << "}" << std::endl;
    }

}

void MxMeshRenderer::draw() {
    for(CellPtr cell : mesh->cells) {
        if(cell == mesh->rootCell()) {
            continue;
        }

        if(!cell->renderer) {
            cell->renderer = new MagnumCellRenderer{cell};
        }

        ((MagnumCellRenderer*)cell->renderer)->draw(*shader);
    }
}

std::string glErrorString() {
    GLenum error = glGetError();
    switch (error) {
        case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
        case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
        case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
        case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";
        case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW";
        case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
        case GL_INVALID_FRAMEBUFFER_OPERATION: return "GL_INVALID_FRAMEBUFFER_OPERATION";
        case GL_CONTEXT_LOST: return "GL_CONTEXT_LOST";
        default: return "who the fuck knows...";
    }
}



MxMeshRenderer::~MxMeshRenderer() {
    delete shader;
}

MxMeshRenderer& MxMeshRenderer::setMesh(MxMesh* _mesh) {

    mesh = _mesh;

    for(auto cell : mesh->cells) {
        if(cell != mesh->rootCell()) {
            cell->renderer = new MagnumCellRenderer{cell};
        }
    }

    return *this;
}

MxMeshRenderer& MxMeshRenderer::setModelMatrix(const Magnum::Matrix4& mat) {
    modelMat = mat;
    shader->setTransformationProjectionMatrix(projMat * viewMat * modelMat);
    return *this;
}

MxMeshRenderer& MxMeshRenderer::setViewMatrix(const Magnum::Matrix4& mat) {
    viewMat = mat;
    shader->setTransformationProjectionMatrix(projMat * viewMat * modelMat);
    return *this;
}

MxMeshRenderer& MxMeshRenderer::setProjectionMatrix(
        const Magnum::Matrix4& mat) {
    projMat = mat;
    shader->setTransformationProjectionMatrix(projMat * viewMat * modelMat);
    return *this;
}

void MagnumCellRenderer::draw(AbstractShaderProgram& shader) {

    //if(!cell->render) {
    //    return;
    //}

    indexBuffer.unmap();

    void* vertexPtr = vertexBuffer.map<void>(0,
            cell->vertexCount() * ShaderProgram::AttributeSize,
            Buffer::MapFlag::Write|Buffer::MapFlag::InvalidateBuffer);

    cell->vertexAtributeData({}, cell->vertexCount(), ShaderProgram::AttributeSize, vertexPtr);

    //dumpVertex(vertexPtr, cell.vertexCount());

    vertexBuffer.unmap();

    mesh.draw(shader);
}

HRESULT MagnumCellRenderer::invalidate() {

    vertexBuffer.setData({nullptr, cell->vertexCount() * sizeof(VertexAttribute)},
                         BufferUsage::DynamicDraw);

    mesh = Mesh{};

    mesh.setCount(cell->faceCount() * 3)
            .setPrimitive(MeshPrimitive::Triangles)
            .addVertexBuffer(vertexBuffer, 0, ShaderProgram::Position{}, ShaderProgram::Normal{}, ShaderProgram::Color{});
            //.setIndexBuffer(obj.indexBuffer, 0, Mesh::IndexType::UnsignedInt);

    return S_OK;
}

MagnumCellRenderer::~MagnumCellRenderer() {
}
