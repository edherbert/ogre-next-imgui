#pragma once

#include "imgui.h"
#include "ImguiRenderable.h"

#include <OgreSingleton.h>
#include <OgreTextureGpu.h>
#include <OgreFastArray.h>
#include <OgreFrameListener.h>
#include <OgreRenderPassDescriptor.h>


namespace Ogre{
    class PsoCacheHelper;
}

class SceneManager;

class ImguiManager : public Ogre::Singleton < ImguiManager >
{
public:
    ImguiManager();
    ~ImguiManager();

    //call once before using ImGui
    void init(Ogre::CompositorWorkspace* compositor);

    //call every frame before using any Imgui functions
    void newFrame(float deltaTime);
    //call at the end of the frame, the gui gets rendered inside of this function
    void render();

    void updateProjectionMatrix(float width, float height);

    void shutdown();


    /** Override standard Singleton retrieval.
    @remarks
    Why do we do this? Well, it's because the Singleton
    implementation is in a .h file, which means it gets compiled
    into anybody who includes it. This is needed for the
    Singleton template to work, but we actually only want it
    compiled into the implementation of the class based on the
    Singleton, not all of them. If we don't change this, we get
    link errors when trying to use the Singleton-based class from
    an outside dll.
    @par
    This method just delegates to the template version anyway,
    but the implementation stays in this single compilation unit,
    preventing link errors.
    */
    static ImguiManager& getSingleton(void);
    /** Override standard Singleton retrieval.
    @remarks
    Why do we do this? Well, it's because the Singleton
    implementation is in a .h file, which means it gets compiled
    into anybody who includes it. This is needed for the
    Singleton template to work, but we actually only want it
    compiled into the implementation of the class based on the
    Singleton, not all of them. If we don't change this, we get
    link errors when trying to use the Singleton-based class from
    an outside dll.
    @par
    This method just delegates to the template version anyway,
    but the implementation stays in this single compilation unit,
    preventing link errors.
    */
    static ImguiManager* getSingletonPtr(void);

private:
    void createFontTexture();
    void createMaterial();

    Ogre::FastArray<ImguiRenderable*> mRenderables;

    Ogre::PsoCacheHelper* mPSOCache;

    Ogre::SceneManager *mSceneMgr;

    Ogre::Pass *mPass;
    Ogre::TextureGpu *mFontTex;
    const Ogre::HlmsSamplerblock* mSamplerblock;

    int mLastRenderedFrame;
    bool mFrameEnded;

    bool mVulkan;

    float mPrevWidth, mPrevHeight;

    Ogre::CompositorWorkspace *mCompositor;

    Ogre::RenderPassDescriptor *renderPassDesc = nullptr;
};
