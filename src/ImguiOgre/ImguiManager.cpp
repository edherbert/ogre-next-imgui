#include "ImguiManager.h"

#include <CommandBuffer/OgreCbDrawCall.h>
#include <Compositor/OgreCompositorWorkspace.h>
#include <Compositor/Pass/OgreCompositorPass.h>
#include <OgreHighLevelGpuProgramManager.h>
#include <OgreHlmsDatablock.h>
#include <OgreMaterialManager.h>
#include <OgrePass.h>
#include <OgrePsoCacheHelper.h>
#include <OgreRenderSystem.h>
#include <OgreRoot.h>
#include <OgreSceneManager.h>
#include <OgreTechnique.h>
#include <OgreTextureBox.h>
#include <OgreTextureGpuManager.h>
#include <OgreUnifiedHighLevelGpuProgram.h>
#include <OgreViewport.h>
#include <OgreHlmsManager.h>

template <>
ImguiManager* Ogre::Singleton<ImguiManager>::msSingleton = 0;

ImguiManager* ImguiManager::getSingletonPtr(void) {
    if (!msSingleton) {
        msSingleton = new ImguiManager();
    }
    return msSingleton;
}
ImguiManager& ImguiManager::getSingleton(void) {
    if (!msSingleton) {
        msSingleton = new ImguiManager();
    }
    return (*msSingleton);
}

ImguiManager::ImguiManager() : mSceneMgr(0),
                               mLastRenderedFrame(4),
                               mFrameEnded(true),
                               mPrevWidth(0),
                               mPrevHeight(0),
                               mVulkan(false) {
}
ImguiManager::~ImguiManager() {

}

void ImguiManager::shutdown() {
    while (mRenderables.size() > 0) {
        delete mRenderables.back();
        mRenderables.pop_back();
    }
    delete mPSOCache;
    Ogre::Root::getSingletonPtr()->getHlmsManager()->destroySamplerblock(mSamplerblock);

    mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager()->destroyTexture(mFontTex);
}

void ImguiManager::init(Ogre::CompositorWorkspace* compositor) {
    mSceneMgr = compositor->getSceneManager();
    mCompositor = compositor;

    mPSOCache = new Ogre::PsoCacheHelper(mSceneMgr->getDestinationRenderSystem());
    Ogre::HlmsSamplerblock s;
    mSamplerblock = Ogre::Root::getSingletonPtr()->getHlmsManager()->getSamplerblock(s);

    Ogre::String renderSystemName = mSceneMgr->getDestinationRenderSystem()->getName();
    mVulkan = (renderSystemName == "Vulkan Rendering Subsystem");

    createFontTexture();
    createMaterial();
}

void ImguiManager::newFrame(float deltaTime) {
    mFrameEnded = false;
    ImGuiIO& io = ImGui::GetIO();

    io.DeltaTime = deltaTime;

    // just some defaults so it doesn't crash
    float width = 400.f;
    float height = 400.f;

    // might not exist if this got called before the rendering loop
    Ogre::TextureGpu* renderTarget = mCompositor->getFinalTarget();
    if (renderTarget) {
        width = renderTarget->getWidth();
        height = renderTarget->getHeight();
    }
    if (mPrevWidth != width || mPrevHeight != height) {
        mPrevWidth = width;
        mPrevHeight = height;

        io.DisplaySize = ImVec2(width, height);
        updateProjectionMatrix(width, height);
    }

    // Start the frame
    ImGui::NewFrame();
}

void ImguiManager::updateProjectionMatrix(float width, float height) {
    Ogre::Matrix4 projMatrix(
        2.0f / width, 0.0f, 0.0f, -1.0f,
        0.0f, (mVulkan ? 2.0f : -2.0f) / height, 0.0f, (mVulkan ? -1.0f : 1.0f),
        0.0f, 0.0f, -1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    mPass->getVertexProgramParameters()->setNamedConstant("ProjectionMatrix", projMatrix);

    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize = ImVec2(width, height);
}

void ImguiManager::render() {
#ifdef __APPLE__
    @autoreleasepool {
#endif

    Ogre::RenderSystem* renderSystem = mSceneMgr->getDestinationRenderSystem();

    if (!renderPassDesc) {
        renderPassDesc = renderSystem->createRenderPassDescriptor();
        renderPassDesc->mColour[0].texture = mCompositor->getFinalTarget();
        renderPassDesc->mColour[0].loadAction = Ogre::LoadAction::Load;
        renderPassDesc->mColour[0].storeAction = Ogre::StoreAction::StoreAndMultisampleResolve;
        renderPassDesc->entriesModified(Ogre::RenderPassDescriptor::All);
    }
    Ogre::Vector4 viewportSize(0, 0, 1, 1);
    Ogre::Vector4 scissors(0, 0, 1, 1);
    renderSystem->beginRenderPassDescriptor(renderPassDesc, mCompositor->getFinalTarget(), 0, &viewportSize, &scissors, 1, false, false);
    renderSystem->executeRenderPassDescriptorDelayedActions();

    // Cancel rendering if not necessary
    // or if newFrame didn't got called
    if (mFrameEnded)
        return;
    mFrameEnded = true;

    int currentFrame = ImGui::GetFrameCount();
    if (currentFrame == mLastRenderedFrame) {
        return;
    }
    mLastRenderedFrame = currentFrame;

    // Tell ImGui to create the buffers
    ImGui::Render();

    if (currentFrame <= 1) {
        // Lots of stuff can be done only once for the sake of efficiency.

        mPSOCache->clearState();

        mPSOCache->setRenderTarget(renderPassDesc);  // dark_sylinc's advice on setting rendertarget, which looks like the renderwindow obj)

        const Ogre::HlmsBlendblock* blendblock = mPass->getBlendblock();
        const Ogre::HlmsMacroblock* macroblock = mPass->getMacroblock();
        mPSOCache->setMacroblock(macroblock);
        mPSOCache->setBlendblock(blendblock);
        mPSOCache->setVertexShader(const_cast<Ogre::GpuProgramPtr&>(mPass->getVertexProgram()));
        mPSOCache->setPixelShader(const_cast<Ogre::GpuProgramPtr&>(mPass->getFragmentProgram()));
    }

    ImDrawData* drawData = ImGui::GetDrawData();
    int numberDraws = 0;

    int vpWidth = mCompositor->getFinalTarget()->getWidth();
    int vpHeight = mCompositor->getFinalTarget()->getHeight();

    // iterate through all lists (at the moment every window has its own)
    for (int n = 0; n < drawData->CmdListsCount; n++) {
        const ImDrawList* drawList = drawData->CmdLists[n];
        const ImDrawVert* vtxBuf = drawList->VtxBuffer.Data;
        const ImDrawIdx* idxBuf = drawList->IdxBuffer.Data;

        unsigned int startIdx = 0;

        for (int i = 0; i < drawList->CmdBuffer.Size; i++) {
            // create renderables if necessary
            // This creates a new one each time it notices the list is bigger than necessary.
            if (i >= mRenderables.size()) {
                mRenderables.push_back(new ImguiRenderable());
            }

            // update their vertex buffers
            const ImDrawCmd* drawCmd = &drawList->CmdBuffer[i];
            mRenderables[i]->updateVertexData(vtxBuf, &idxBuf[startIdx], drawList->VtxBuffer.Size, drawCmd->ElemCount);

            // Set scissoring
            int scLeft = static_cast<int>(drawCmd->ClipRect.x);  // Obtain bounds
            int scTop = static_cast<int>(drawCmd->ClipRect.y);
            int scRight = static_cast<int>(drawCmd->ClipRect.z);
            int scBottom = static_cast<int>(drawCmd->ClipRect.w);

            scLeft = scLeft < 0 ? 0 : (scLeft > vpWidth ? vpWidth : scLeft);  // Clamp bounds to viewport dimensions
            scRight = scRight < 0 ? 0 : (scRight > vpWidth ? vpWidth : scRight);
            scTop = scTop < 0 ? 0 : (scTop > vpHeight ? vpHeight : scTop);
            scBottom = scBottom < 0 ? 0 : (scBottom > vpHeight ? vpHeight : scBottom);

            float left = (float)scLeft / (float)vpWidth;
            float top = (float)scTop / (float)vpHeight;
            float width = (float)(scRight - scLeft) / (float)vpWidth;
            float height = (float)(scBottom - scTop) / (float)vpHeight;

            scissors = Ogre::Vector4(left, top, width, height);
            renderSystem->beginRenderPassDescriptor(renderPassDesc, mCompositor->getFinalTarget(), 0, &viewportSize, &scissors, 1, false, false);
            renderSystem->executeRenderPassDescriptorDelayedActions();

            Ogre::v1::RenderOperation renderOp;
            mRenderables[i]->getRenderOperation(renderOp, false);

            bool enablePrimitiveRestart = true;  // tried both true and false...no change

            Ogre::VertexElement2VecVec vertexElements = renderOp.vertexData->vertexDeclaration->convertToV2();
            mPSOCache->setVertexFormat(vertexElements,
                                       renderOp.operationType,
                                       enablePrimitiveRestart);

            Ogre::HlmsPso* pso = mPSOCache->getPso();
            mSceneMgr->getDestinationRenderSystem()->_setPipelineStateObject(pso);

            mSceneMgr->getDestinationRenderSystem()->bindGpuProgramParameters(Ogre::GPT_VERTEX_PROGRAM, mPass->getVertexProgramParameters(), Ogre::GPV_ALL);

            Ogre::TextureGpu* texGpu = (Ogre::TextureGpu*)drawCmd->TextureId;
            mSceneMgr->getDestinationRenderSystem()->_setTexture(0, texGpu, false);
            mSceneMgr->getDestinationRenderSystem()->_setHlmsSamplerblock(0, mSamplerblock);

            Ogre::v1::CbRenderOp op(renderOp);
            mSceneMgr->getDestinationRenderSystem()->_setRenderOperation(&op);

            mSceneMgr->getDestinationRenderSystem()->_render(renderOp);

            // increase start index of indexbuffer
            startIdx += drawCmd->ElemCount;
            numberDraws++;
        }
    }

    // reset Scissors
    // vp->setScissors(0, 0, 1, 1);
    // TODO: uncommented //mSceneMgr->getDestinationRenderSystem()->_setViewport(vp);

    // delete unused renderables
    while (mRenderables.size() > numberDraws) {
        delete mRenderables.back();
        mRenderables.pop_back();
    }

    renderSystem->endRenderPassDescriptor();
#ifdef __APPLE__
    }
#endif
}

void ImguiManager::createMaterial() {
    static const char* vertexShaderSrcD3D11 =
        {
            "uniform float4x4 ProjectionMatrix;\n"
            "struct VS_INPUT\n"
            "{\n"
            "float2 pos : POSITION;\n"
            "float4 col : COLOR0;\n"
            "float2 uv  : TEXCOORD0;\n"
            "};\n"
            "struct PS_INPUT\n"
            "{\n"
            "float4 pos : SV_POSITION;\n"
            "float4 col : COLOR0;\n"
            "float2 uv  : TEXCOORD0;\n"
            "};\n"
            "PS_INPUT main(VS_INPUT input)\n"
            "{\n"
            "PS_INPUT output;\n"
            "output.pos = mul(ProjectionMatrix, float4(input.pos.xy, 0.f, 1.f));\n"
            "output.col = input.col;\n"
            "output.uv  = input.uv;\n"
            "return output;\n"
            "}"};
    static const char* pixelShaderSrcD3D11 =
        {
            "struct PS_INPUT\n"
            "{\n"
            "float4 pos : SV_POSITION;\n"
            "float4 col : COLOR0;\n"
            "float2 uv  : TEXCOORD0;\n"
            "};\n"
            "sampler sampler0: register(s0);\n"
            "Texture2D texture0: register(t0);\n"
            "\n"
            "float4 main(PS_INPUT input) : SV_Target\n"
            "{\n"
            "float4 out_col = input.col * texture0.Sample(sampler0, input.uv);\n"
            "return out_col; \n"
            "}"};

    static const char* vertexShaderSrcGLSL =
        {
            "#version 150\n"
            "uniform mat4 ProjectionMatrix; \n"
            "in vec2 vertex;\n"
            "in vec2 uv0;\n"
            "in vec4 colour;\n"
            "out vec2 Texcoord;\n"
            "out vec4 col;\n"
            "void main()\n"
            "{\n"
            "gl_Position = ProjectionMatrix* vec4(vertex.xy, 0.f, 1.f);\n"
            "Texcoord  = uv0;\n"
            "col = colour;\n"
            "}"};
    static const char* pixelShaderSrcGLSL =
        {
            "#version 150\n"
            "in vec2 Texcoord;\n"
            "in vec4 col;\n"
            "uniform sampler2D sampler0;\n"
            "out vec4 out_col;\n"
            "void main()\n"
            "{\n"
            "out_col = col * texture(sampler0, Texcoord);\n"
            "}"};
    static const char* vertexShaderSrcVK =
        {
            "vulkan( layout( ogre_P0 ) uniform Params { )\n"
            "    uniform mat4 ProjectionMatrix; \n"
            "vulkan( }; )\n"
            "vulkan_layout( OGRE_POSITION ) in vec2 vertex;\n"
            "vulkan_layout( OGRE_TEXCOORD0 ) in vec2 Texcoord;\n"
            "vulkan_layout( OGRE_DIFFUSE ) in vec4 colour;\n"
            "vulkan_layout( location = 1 )\n"
            "out block\n"
            "{\n"
            "    vec2 uv0;\n"
            "    vec4 col;\n"
            "} outVs;\n"
            "void main()\n"
            "{\n"
            "    gl_Position = ProjectionMatrix* vec4(vertex.xy, 0.f, 1.f);\n"
            "    outVs.uv0 = Texcoord;\n"
            "    outVs.col = colour;\n"
            "}"};
    static const char* pixelShaderSrcVK =
        {
            "vulkan_layout( ogre_t0 ) uniform texture2D sampler0;\n"
            "vulkan( layout( ogre_s0 ) uniform sampler texSampler );\n"
            "vulkan_layout( location = 0 )\n"
            "out vec4 out_col;\n"
            "vulkan_layout( location = 1 )\n"
            "in block\n"
            "{\n"
            "    vec2 uv0;\n"
            "    vec4 col;\n"
            "} inPs;\n"
            "void main()\n"
            "{\n"
            "    out_col = inPs.col * texture( vkSampler2D( sampler0, texSampler ), inPs.uv0 );"
            "}"};
    static const char* fragmentShaderSrcMetal =
        {
            "#include <metal_stdlib>\n"
            "using namespace metal;\n"
            "\n"
            "struct VertexOut {\n"
            "    float4 position [[position]];\n"
            "    float2 texCoords;\n"
            "    float4 colour;\n"
            "};\n"
            "\n"
            "fragment float4 main_metal(VertexOut in [[stage_in]],\n"
            "                             texture2d<float> texture [[texture(0)]]) {\n"
            "    constexpr sampler linearSampler(coord::normalized, min_filter::linear, mag_filter::linear, mip_filter::linear);\n"
            "    float4 texColour = texture.sample(linearSampler, in.texCoords);\n"
            "    return in.colour * texColour;\n"
            "}\n"};

    static const char* vertexShaderSrcMetal =
        {
            "#include <metal_stdlib>\n"
            "using namespace metal;\n"
            "\n"
            "struct Constant {\n"
            "    float4x4 ProjectionMatrix;\n"
            "};\n"
            "\n"
            "struct VertexIn {\n"
            "    float2 position  [[attribute(VES_POSITION)]];\n"
            "    float2 texCoords [[attribute(VES_TEXTURE_COORDINATES0)]];\n"
            "    float4 colour     [[attribute(VES_DIFFUSE)]];\n"
            "};\n"
            "\n"
            "struct VertexOut {\n"
            "    float4 position [[position]];\n"
            "    float2 texCoords;\n"
            "    float4 colour;\n"
            "};\n"
            "\n"
            "vertex VertexOut vertex_main(VertexIn in                 [[stage_in]],\n"
            "                             constant Constant &uniforms [[buffer(PARAMETER_SLOT)]]) {\n"
            "    VertexOut out;\n"
            "    out.position = uniforms.ProjectionMatrix * float4(in.position, 0, 1);\n"

            "    out.texCoords = in.texCoords;\n"
            "    out.colour = in.colour;\n"

            "    return out;\n"
            "}\n"};

    // create the default shadows material
    Ogre::HighLevelGpuProgramManager& mgr = Ogre::HighLevelGpuProgramManager::getSingleton();

    Ogre::HighLevelGpuProgramPtr vertexShaderUnified = mgr.getByName("imgui/VP");
    Ogre::HighLevelGpuProgramPtr pixelShaderUnified = mgr.getByName("imgui/FP");

    Ogre::HighLevelGpuProgramPtr vertexShaderD3D11 = mgr.getByName("imgui/VP/D3D11");
    Ogre::HighLevelGpuProgramPtr pixelShaderD3D11 = mgr.getByName("imgui/FP/D3D11");

    Ogre::HighLevelGpuProgramPtr vertexShaderGL = mgr.getByName("imgui/VP/GL150");
    Ogre::HighLevelGpuProgramPtr pixelShaderGL = mgr.getByName("imgui/FP/GL150");

    Ogre::HighLevelGpuProgramPtr vertexShaderVK = mgr.getByName("imgui/VP/VK");
    Ogre::HighLevelGpuProgramPtr pixelShaderVK = mgr.getByName("imgui/FP/VK");

    Ogre::HighLevelGpuProgramPtr vertexShaderMetal = mgr.getByName("imgui/VP/Metal");
    Ogre::HighLevelGpuProgramPtr pixelShaderMetal = mgr.getByName("imgui/FP/Metal");

    if (vertexShaderUnified.isNull()) {
        vertexShaderUnified = mgr.createProgram("imgui/VP", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "unified", Ogre::GPT_VERTEX_PROGRAM);
    }
    if (pixelShaderUnified.isNull()) {
        pixelShaderUnified = mgr.createProgram("imgui/FP", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME, "unified", Ogre::GPT_FRAGMENT_PROGRAM);
    }

    Ogre::UnifiedHighLevelGpuProgram* vertexShaderPtr = static_cast<Ogre::UnifiedHighLevelGpuProgram*>(vertexShaderUnified.get());
    Ogre::UnifiedHighLevelGpuProgram* pixelShaderPtr = static_cast<Ogre::UnifiedHighLevelGpuProgram*>(pixelShaderUnified.get());

    if (vertexShaderD3D11.isNull()) {
        vertexShaderD3D11 = mgr.createProgram("imgui/VP/D3D11", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            "hlsl", Ogre::GPT_VERTEX_PROGRAM);
        vertexShaderD3D11->setParameter("target", "vs_4_0");
        vertexShaderD3D11->setParameter("entry_point", "main");
        vertexShaderD3D11->setSource(vertexShaderSrcD3D11);
        vertexShaderD3D11->load();

        vertexShaderPtr->addDelegateProgram(vertexShaderD3D11->getName());
    }
    if (pixelShaderD3D11.isNull()) {
        pixelShaderD3D11 = mgr.createProgram("imgui/FP/D3D11", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            "hlsl", Ogre::GPT_FRAGMENT_PROGRAM);
        pixelShaderD3D11->setParameter("target", "ps_4_0");
        pixelShaderD3D11->setParameter("entry_point", "main");
        pixelShaderD3D11->setSource(pixelShaderSrcD3D11);
        pixelShaderD3D11->load();

        pixelShaderPtr->addDelegateProgram(pixelShaderD3D11->getName());
    }

    if (vertexShaderMetal.isNull()) {
        vertexShaderMetal = mgr.createProgram("imgui/VP/Metal", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            "metal", Ogre::GPT_VERTEX_PROGRAM);
        vertexShaderMetal->setParameter("entry_point", "vertex_main");
        vertexShaderMetal->setSource(vertexShaderSrcMetal);
        vertexShaderMetal->load();
        vertexShaderPtr->addDelegateProgram(vertexShaderMetal->getName());
    }
    if (pixelShaderMetal.isNull()) {
        pixelShaderMetal = mgr.createProgram("imgui/FP/Metal", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                            "metal", Ogre::GPT_FRAGMENT_PROGRAM);
        vertexShaderMetal->setParameter("entry_point", "fragment_main");
        pixelShaderMetal->setSource(fragmentShaderSrcMetal);
        pixelShaderMetal->load();
        pixelShaderPtr->addDelegateProgram(pixelShaderMetal->getName());
    }

    if (vertexShaderGL.isNull()) {
        vertexShaderGL = mgr.createProgram("imgui/VP/GL150", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                        "glsl", Ogre::GPT_VERTEX_PROGRAM);
        vertexShaderGL->setSource(vertexShaderSrcGLSL);
        vertexShaderGL->load();
        vertexShaderPtr->addDelegateProgram(vertexShaderGL->getName());
    }
    if (pixelShaderGL.isNull()) {
        pixelShaderGL = mgr.createProgram("imgui/FP/GL150", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                        "glsl", Ogre::GPT_FRAGMENT_PROGRAM);
        pixelShaderGL->setSource(pixelShaderSrcGLSL);
        pixelShaderGL->load();

        pixelShaderPtr->addDelegateProgram(pixelShaderGL->getName());
    }

    if (vertexShaderVK.isNull()) {
        vertexShaderVK = mgr.createProgram("imgui/VP/vulkan", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                        "glslvk", Ogre::GPT_VERTEX_PROGRAM);
        vertexShaderVK->setSource(vertexShaderSrcVK);
        vertexShaderVK->setPrefabRootLayout(Ogre::PrefabRootLayout::Standard);
        vertexShaderVK->load();
        vertexShaderPtr->addDelegateProgram(vertexShaderVK->getName());
    }
    if (pixelShaderVK.isNull()) {
        pixelShaderVK = mgr.createProgram("imgui/FP/vulkan", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                                        "glslvk", Ogre::GPT_FRAGMENT_PROGRAM);
        pixelShaderVK->setSource(pixelShaderSrcVK);
        pixelShaderVK->setPrefabRootLayout(Ogre::PrefabRootLayout::Standard);
        pixelShaderVK->load();

        pixelShaderPtr->addDelegateProgram(pixelShaderVK->getName());
    }

    Ogre::MaterialPtr imguiMaterial = Ogre::MaterialManager::getSingleton().create("imgui/material", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME);
    mPass = imguiMaterial->getTechnique(0)->getPass(0);
    mPass->setFragmentProgram("imgui/FP");
    mPass->setVertexProgram("imgui/VP");

    Ogre::HlmsBlendblock blendblock(*mPass->getBlendblock());
    blendblock.mSourceBlendFactor = Ogre::SBF_SOURCE_ALPHA;
    blendblock.mDestBlendFactor = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;
    blendblock.mSourceBlendFactorAlpha = Ogre::SBF_ONE_MINUS_SOURCE_ALPHA;
    blendblock.mDestBlendFactorAlpha = Ogre::SBF_ZERO;
    blendblock.mBlendOperation = Ogre::SBO_ADD;
    blendblock.mBlendOperationAlpha = Ogre::SBO_ADD;
    blendblock.mSeparateBlend = true;
    blendblock.mIsTransparent = true;

    Ogre::HlmsMacroblock macroblock(*mPass->getMacroblock());
    macroblock.mCullMode = Ogre::CULL_NONE;
    macroblock.mDepthFunc = Ogre::CMPF_ALWAYS_PASS;
    macroblock.mDepthCheck = false;
    macroblock.mDepthWrite = false;
    macroblock.mScissorTestEnabled = true;

    mPass->setBlendblock(blendblock);
    mPass->setMacroblock(macroblock);

    mPass->createTextureUnitState()->setTexture(mFontTex);
}

void ImguiManager::createFontTexture() {
    // Build texture atlas
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    unsigned char* pixels;
    int width, height;

    io.Fonts->AddFontDefault();
    io.Fonts->GetTexDataAsRGBA32(&pixels, &width, &height);

    Ogre::TextureGpuManager* textureManager = mSceneMgr->getDestinationRenderSystem()->getTextureGpuManager();

    mFontTex = textureManager->createTexture("imgui/fontTex", Ogre::GpuPageOutStrategy::AlwaysKeepSystemRamCopy, 0, Ogre::TextureTypes::Type2D);
    mFontTex->setPixelFormat(Ogre::PixelFormatGpu::PFG_RGBA8_UNORM);
    mFontTex->setResolution(width, height);

    io.Fonts->SetTexID(mFontTex);

    Ogre::Image2 image;

    int sizeInBytes = width * height * 4;
    Ogre::uint8* data = reinterpret_cast<Ogre::uint8*>(OGRE_MALLOC_SIMD(sizeInBytes, Ogre::MEMCATEGORY_GENERAL));
    image.loadDynamicImage(data, width, height, 1u, Ogre::TextureTypes::Type2D, mFontTex->getPixelFormat(), true, 1U);
    memcpy(data, pixels, sizeInBytes);

    bool canUseSynchronousUpload =
        mFontTex->getNextResidencyStatus() == Ogre::GpuResidency::Resident &&
        mFontTex->isDataReady();
    if (canUseSynchronousUpload) {
        // If canUseSynchronousUpload is false, you can use areaTex->waitForData()
        // to still use sync method (assuming the texture is resident)
        image.uploadTo(mFontTex, 0, mFontTex->getNumMipmaps() - 1u);
    } else {
        // Asynchronous is preferred due to being done in the background. But the switch
        // Resident -> OnStorage -> Resident may cause undesired effects, so we
        // show how to do it synchronously

        // Tweak via _setAutoDelete so the internal data is copied as a pointer
        // instead of performing a deep copy of the data; while leaving the responsability
        // of freeing memory to imagePtr instead.
        image._setAutoDelete(false);
        Ogre::Image2* imagePtr = new Ogre::Image2(image);
        imagePtr->_setAutoDelete(true);

        if (mFontTex->getNextResidencyStatus() == Ogre::GpuResidency::Resident)
            mFontTex->scheduleTransitionTo(Ogre::GpuResidency::OnStorage);
        // Ogre will call "delete imagePtr" when done, because we're passing
        // true to autoDeleteImage argument in scheduleTransitionTo
        mFontTex->scheduleTransitionTo(Ogre::GpuResidency::Resident, imagePtr, true);
    }
}
