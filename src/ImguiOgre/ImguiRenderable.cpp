#include <imgui.h>

#include "ImguiRenderable.h"

#include <OgreHardwareBufferManager.h>
#include <OgreHardwareVertexBuffer.h>
#include <OgreHardwareIndexBuffer.h>


ImguiRenderable::ImguiRenderable()
{
    mVertexBufferSize = 0;
    mIndexBufferSize = 0;

    // use identity projection and view matrices
    mUseIdentityProjection = true;
    mUseIdentityView = true;

    //By default we want ImguiRenderables to still work in wireframe mode
    mPolygonModeOverrideable = false;

    mRenderOp.vertexData = OGRE_NEW Ogre::v1::VertexData(Ogre::v1::HardwareBufferManager::getSingletonPtr());
    mRenderOp.indexData = OGRE_NEW Ogre::v1::IndexData();

    mRenderOp.vertexData->vertexCount = 0;
    mRenderOp.vertexData->vertexStart = 0;

    mRenderOp.indexData->indexCount = 0;
    mRenderOp.indexData->indexStart = 0;
    //mRenderOp.operationType = Ogre::v1::RenderOperation::OT_TRIANGLE_LIST;
    mRenderOp.operationType = Ogre::OperationType::OT_TRIANGLE_LIST;
    mRenderOp.useIndexes = true;
    mRenderOp.useGlobalInstancingVertexBufferIsAvailable = false;

    Ogre::v1::VertexDeclaration* decl = mRenderOp.vertexData->vertexDeclaration;

    // vertex declaration
    size_t offset = 0;
    decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_POSITION);
    offset += Ogre::v1::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
    decl->addElement(0, offset, Ogre::VET_FLOAT2, Ogre::VES_TEXTURE_COORDINATES, 0);
    offset += Ogre::v1::VertexElement::getTypeSize(Ogre::VET_FLOAT2);
    decl->addElement(0, offset, Ogre::VET_COLOUR, Ogre::VES_DIFFUSE);
}

ImguiRenderable::~ImguiRenderable()
{
    OGRE_DELETE mRenderOp.vertexData;
    OGRE_DELETE mRenderOp.indexData;
}

void ImguiRenderable::updateVertexData(const ImDrawVert* vtxBuf, const ImDrawIdx* idxBuf, unsigned int vtxCount, unsigned int idxCount)
{
#ifdef __APPLE__
    @autoreleasepool{
#endif
    Ogre::v1::VertexBufferBinding* bind = mRenderOp.vertexData->vertexBufferBinding;

    if (bind->getBindings().empty() || mVertexBufferSize != vtxCount)
    {
        mVertexBufferSize = vtxCount;

        bind->setBinding(0, Ogre::v1::HardwareBufferManager::getSingleton().createVertexBuffer(sizeof(ImDrawVert), mVertexBufferSize, Ogre::v1::HardwareBuffer::HBU_WRITE_ONLY));
    }
    if (mRenderOp.indexData->indexBuffer.isNull() || mIndexBufferSize != idxCount)
    {
        mIndexBufferSize = idxCount;

        mRenderOp.indexData->indexBuffer =
            Ogre::v1::HardwareBufferManager::getSingleton().createIndexBuffer(Ogre::v1::HardwareIndexBuffer::IT_16BIT, mIndexBufferSize, Ogre::v1::HardwareBuffer::HBU_WRITE_ONLY);
    }

    // Copy all vertices
    ImDrawVert* vtxDst = (ImDrawVert*)(bind->getBuffer(0)->lock(Ogre::v1::HardwareBuffer::HBL_DISCARD));
    ImDrawIdx* idxDst = (ImDrawIdx*)(mRenderOp.indexData->indexBuffer->lock(Ogre::v1::HardwareBuffer::HBL_DISCARD));

    memcpy(vtxDst, vtxBuf, mVertexBufferSize * sizeof(ImDrawVert));
    memcpy(idxDst, idxBuf, mIndexBufferSize * sizeof(ImDrawIdx));

    mRenderOp.vertexData->vertexStart = 0;
    mRenderOp.vertexData->vertexCount = vtxCount;
    mRenderOp.indexData->indexStart = 0;
    mRenderOp.indexData->indexCount = idxCount;


    bind->getBuffer(0)->unlock();
    mRenderOp.indexData->indexBuffer->unlock();

#ifdef __APPLE__
    }
#endif
}


void ImguiRenderable::getWorldTransforms(Ogre::Matrix4* xform) const
{
    *xform = Ogre::Matrix4::IDENTITY;
}

void ImguiRenderable::getRenderOperation(Ogre::v1::RenderOperation& op, bool casterPass)
{
    op = mRenderOp;
}

const Ogre::LightList& ImguiRenderable::getLights(void) const
{
    static const Ogre::LightList l;
    return l;
}
