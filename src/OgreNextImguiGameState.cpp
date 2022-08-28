
#include "OgreNextImguiGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

#include "OgreMesh.h"
#include "OgreMeshManager.h"
#include "OgreMeshManager2.h"
#include "OgreMesh2.h"

#include "OgreCamera.h"

#include "OgreHlmsPbsDatablock.h"
#include "OgreHlmsSamplerblock.h"

#include "OgreRoot.h"
#include "OgreHlmsManager.h"
#include "OgreHlmsPbs.h"

#include "OgreWindow.h"
#include "SdlInputHandler.h"

#include "OgreLogManager.h"

#include "ImguiOgre/ImguiManager.h"

using namespace Demo;

namespace Demo
{

    class ImguiFrameListener : public Ogre::FrameListener{
        bool frameRenderingQueued(const Ogre::FrameEvent& evt){
            ImguiManager::getSingletonPtr()->render();
            return true;
        }
    };



    OgreNextImguiGameState::OgreNextImguiGameState( const Ogre::String &helpDescription ) :
        TutorialGameState( helpDescription )
    {

    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::createScene01(void)
    {
        // Load a mesh to show Ogre rendering something.
        {
            Ogre::SceneManager* sceneManager = mGraphicsSystem->getSceneManager();
            Ogre::v1::MeshPtr v1Mesh;
            Ogre::MeshPtr v2Mesh;
            v1Mesh = Ogre::v1::MeshManager::getSingleton().load(
                        "athene.mesh", Ogre::ResourceGroupManager::AUTODETECT_RESOURCE_GROUP_NAME,
                        Ogre::v1::HardwareBuffer::HBU_STATIC, Ogre::v1::HardwareBuffer::HBU_STATIC );
            bool halfPosition   = true;
            bool halfUVs        = true;
            bool useQtangents   = true;
            v2Mesh = Ogre::MeshManager::getSingleton().createByImportingV1(
                        "athene.mesh Imported", Ogre::ResourceGroupManager::DEFAULT_RESOURCE_GROUP_NAME,
                        v1Mesh.get(), halfPosition, halfUVs, useQtangents );
            v1Mesh->unload();
            Ogre::Item *item = sceneManager->createItem( "athene.mesh Imported",
                                                         Ogre::ResourceGroupManager::
                                                         AUTODETECT_RESOURCE_GROUP_NAME,
                                                         Ogre::SCENE_DYNAMIC );
            Ogre::SceneNode *sceneNode = sceneManager->getRootSceneNode( Ogre::SCENE_DYNAMIC )->
                    createChildSceneNode( Ogre::SCENE_DYNAMIC );
            sceneNode->attachObject( item );
            sceneNode->scale( 0.08f, 0.08f, 0.08f );

            Ogre::Light *light = sceneManager->createLight();
            Ogre::SceneNode *lightNode = sceneManager->getRootSceneNode()->createChildSceneNode();
            lightNode->attachObject( light );
            light->setPowerScale( Ogre::Math::PI );
            light->setType( Ogre::Light::LT_DIRECTIONAL );
            light->setDirection( Ogre::Vector3( -1, -1, -1 ).normalisedCopy() );
        }

        //Register imgui for rendering
        Ogre::Root::getSingleton().addFrameListener(new ImguiFrameListener());
        //initialise with your target workspace.
        ImguiManager::getSingleton().init(mGraphicsSystem->getCompositorWorkspace());


        //Setup imgui key codes for SDL2
        {
            ImGuiIO& io = ImGui::GetIO();
            io.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
            io.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
            io.BackendPlatformName = "imgui_impl_ogre_next_sdl";

            // Keyboard mapping. ImGui will use those indices to peek into the io.KeysDown[] array.
            io.KeyMap[ImGuiKey_Tab] = SDL_SCANCODE_TAB;
            io.KeyMap[ImGuiKey_LeftArrow] = SDL_SCANCODE_LEFT;
            io.KeyMap[ImGuiKey_RightArrow] = SDL_SCANCODE_RIGHT;
            io.KeyMap[ImGuiKey_UpArrow] = SDL_SCANCODE_UP;
            io.KeyMap[ImGuiKey_DownArrow] = SDL_SCANCODE_DOWN;
            io.KeyMap[ImGuiKey_PageUp] = SDL_SCANCODE_PAGEUP;
            io.KeyMap[ImGuiKey_PageDown] = SDL_SCANCODE_PAGEDOWN;
            io.KeyMap[ImGuiKey_Home] = SDL_SCANCODE_HOME;
            io.KeyMap[ImGuiKey_End] = SDL_SCANCODE_END;
            io.KeyMap[ImGuiKey_Insert] = SDL_SCANCODE_INSERT;
            io.KeyMap[ImGuiKey_Delete] = SDL_SCANCODE_DELETE;
            io.KeyMap[ImGuiKey_Backspace] = SDL_SCANCODE_BACKSPACE;
            io.KeyMap[ImGuiKey_Space] = SDL_SCANCODE_SPACE;
            io.KeyMap[ImGuiKey_Enter] = SDL_SCANCODE_RETURN;
            io.KeyMap[ImGuiKey_Escape] = SDL_SCANCODE_ESCAPE;
            io.KeyMap[ImGuiKey_A] = SDL_SCANCODE_A;
            io.KeyMap[ImGuiKey_C] = SDL_SCANCODE_C;
            io.KeyMap[ImGuiKey_V] = SDL_SCANCODE_V;
            io.KeyMap[ImGuiKey_X] = SDL_SCANCODE_X;
            io.KeyMap[ImGuiKey_Y] = SDL_SCANCODE_Y;
            io.KeyMap[ImGuiKey_Z] = SDL_SCANCODE_Z;
        }

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::destroyScene()
    {
        ImguiManager::getSingleton().shutdown();
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::update( float timeSinceLast )
    {
        static bool tried = false;
        if( !tried )
        {
            SdlInputHandler *inputHandler = mGraphicsSystem->getInputHandler();
            inputHandler->setGrabMousePointer( false );
            inputHandler->setMouseVisible( true );
            inputHandler->setMouseRelative( false );
            tried = true;
        }

        //Calculate the frame delta time using SDL's helper functions.
        static Uint64 g_Time = 0;
        static Uint64 frequency = SDL_GetPerformanceFrequency();
        Uint64 current_time = SDL_GetPerformanceCounter();
        float deltaTime = g_Time > 0 ? (float)((double)(current_time - g_Time) / frequency) : (float)(1.0f / 60.0f);
        g_Time = current_time;

        ImguiManager::getSingletonPtr()->newFrame(deltaTime);

        //Begin issuing imgui draw calls.
        //Don't do this in the frameRenderingQueued callback,
        //as if any of your logic alters Ogre state you will break things.
        bool show_demo_window = true;
        ImGui::ShowDemoWindow(&show_demo_window);

        TutorialGameState::update( timeSinceLast );
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::generateDebugText( float timeSinceLast, Ogre::String &outText )
    {
        TutorialGameState::generateDebugText( timeSinceLast, outText );
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::mouseMoved( const SDL_Event &arg )
    {
        ImGuiIO& io = ImGui::GetIO();
        if( arg.type == SDL_MOUSEMOTION )
        {
            Ogre::Vector2 mousePos( arg.motion.x, arg.motion.y );
            io.MousePos = ImVec2(mousePos.x, mousePos.y);
        }

        TutorialGameState::mouseMoved( arg );
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::mousePressed( const SDL_MouseButtonEvent &arg, Ogre::uint8 id )
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[arg.button == SDL_BUTTON_LEFT ? 0 : 1] = true;
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::mouseReleased( const SDL_MouseButtonEvent &arg, Ogre::uint8 id )
    {
        ImGuiIO& io = ImGui::GetIO();
        io.MouseDown[arg.button == SDL_BUTTON_LEFT ? 0 : 1] = false;
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::textInput( const SDL_TextInputEvent &arg )
    {
        ImGuiIO& io = ImGui::GetIO();
        io.AddInputCharactersUTF8(arg.text);
    }
    void OgreNextImguiGameState::_keyEvent( const SDL_KeyboardEvent &arg, bool keyPressed )
    {
        ImGuiIO& io = ImGui::GetIO();

        int key = arg.keysym.scancode;
        IM_ASSERT(key >= 0 && key < IM_ARRAYSIZE(io.KeysDown));

        io.KeyShift = ((SDL_GetModState() & KMOD_SHIFT) != 0);
        io.KeyCtrl = ((SDL_GetModState() & KMOD_CTRL) != 0);
        io.KeyAlt = ((SDL_GetModState() & KMOD_ALT) != 0);
        io.KeySuper = ((SDL_GetModState() & KMOD_GUI) != 0);

        io.KeysDown[key] = keyPressed;
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::keyPressed( const SDL_KeyboardEvent &arg )
    {
        _keyEvent( arg, true );

        TutorialGameState::keyPressed( arg );
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        _keyEvent( arg, false );

        TutorialGameState::keyReleased( arg );
    }
}
