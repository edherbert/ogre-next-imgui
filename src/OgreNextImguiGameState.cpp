
#include "OgreNextImguiGameState.h"
#include "CameraController.h"
#include "GraphicsSystem.h"

#include "OgreSceneManager.h"
#include "OgreItem.h"

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
    bool OgreNextImguiGameState::frameRenderingQueued(const Ogre::FrameEvent& evt){
        ImguiManager::getSingletonPtr()->render();

        return true;
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::createScene01(void)
    {
        Ogre::Root::getSingleton().addFrameListener(new ImguiFrameListener());

        ImguiManager::getSingleton().init(mGraphicsSystem->getCompositorWorkspace());

        TutorialGameState::createScene01();
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::destroyScene()
    {
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

        ImguiManager::getSingletonPtr()->newFrame();

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
        float width  = static_cast<float>( mGraphicsSystem->getRenderWindow()->getWidth() );
        float height = static_cast<float>( mGraphicsSystem->getRenderWindow()->getHeight() );

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
    void OgreNextImguiGameState::textEditing( const SDL_TextEditingEvent &arg )
    {
        //colibriManager->setTextEdit( arg.text, arg.start, arg.length );
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::textInput( const SDL_TextInputEvent &arg )
    {
        //colibriManager->setTextInput( arg.text, false );
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::keyPressed( const SDL_KeyboardEvent &arg )
    {
        // const bool isTextInputActive = SDL_IsTextInputActive();
        // const bool isTextMultiline = colibriManager->isTextMultiline();

        // if( (arg.keysym.sym == SDLK_w && !isTextInputActive) || arg.keysym.sym == SDLK_UP )
        //     colibriManager->setKeyDirectionPressed( Colibri::Borders::Top );
        // else if( (arg.keysym.sym == SDLK_s && !isTextInputActive) || arg.keysym.sym == SDLK_DOWN )
        //     colibriManager->setKeyDirectionPressed( Colibri::Borders::Bottom );
        // else if( (arg.keysym.sym == SDLK_a && !isTextInputActive) || arg.keysym.sym == SDLK_LEFT )
        //     colibriManager->setKeyDirectionPressed( Colibri::Borders::Left );
        // else if( (arg.keysym.sym == SDLK_d && !isTextInputActive) || arg.keysym.sym == SDLK_RIGHT )
        //     colibriManager->setKeyDirectionPressed( Colibri::Borders::Right );
        // else if( ((arg.keysym.sym == SDLK_RETURN ||
        //            arg.keysym.sym == SDLK_KP_ENTER) && !isTextMultiline) ||
        //          (arg.keysym.sym == SDLK_SPACE && !isTextInputActive) )
        // {
        //     colibriManager->setKeyboardPrimaryPressed();
        // }
        // else if( isTextInputActive )
        // {
        //     if( (arg.keysym.sym == SDLK_RETURN || arg.keysym.sym == SDLK_KP_ENTER) && isTextMultiline )
        //         colibriManager->setTextSpecialKeyPressed( SDLK_RETURN, arg.keysym.mod );
        //     else
        //     {
        //         colibriManager->setTextSpecialKeyPressed( arg.keysym.sym & ~SDLK_SCANCODE_MASK,
        //                                                   arg.keysym.mod );
        //     }
        // }
    }
    //-----------------------------------------------------------------------------------
    void OgreNextImguiGameState::keyReleased( const SDL_KeyboardEvent &arg )
    {
        // if( (arg.keysym.mod & ~(KMOD_NUM|KMOD_CAPS)) != 0 )
        // {
        //     TutorialGameState::keyReleased( arg );
        //     return;
        // }

        // const bool isTextInputActive = SDL_IsTextInputActive();
        // const bool isTextMultiline = colibriManager->isTextMultiline();

        // if( (arg.keysym.sym == SDLK_w && !isTextInputActive) || arg.keysym.sym == SDLK_UP )
        //     colibriManager->setKeyDirectionReleased( Colibri::Borders::Top );
        // else if( (arg.keysym.sym == SDLK_s && !isTextInputActive) || arg.keysym.sym == SDLK_DOWN )
        //     colibriManager->setKeyDirectionReleased( Colibri::Borders::Bottom );
        // else if( (arg.keysym.sym == SDLK_a && !isTextInputActive) || arg.keysym.sym == SDLK_LEFT )
        //     colibriManager->setKeyDirectionReleased( Colibri::Borders::Left );
        // else if( (arg.keysym.sym == SDLK_d && !isTextInputActive) || arg.keysym.sym == SDLK_RIGHT )
        //     colibriManager->setKeyDirectionReleased( Colibri::Borders::Right );
        // else if( ((arg.keysym.sym == SDLK_RETURN ||
        //            arg.keysym.sym == SDLK_KP_ENTER) && !isTextMultiline) ||
        //          (arg.keysym.sym == SDLK_SPACE && !isTextInputActive) )
        // {
        //     colibriManager->setKeyboardPrimaryReleased();
        // }
        // else if( isTextInputActive )
        // {
        //     if( (arg.keysym.sym == SDLK_RETURN || arg.keysym.sym == SDLK_KP_ENTER) && isTextMultiline )
        //         colibriManager->setTextSpecialKeyReleased( SDLK_RETURN, arg.keysym.mod );
        //     else
        //     {
        //         colibriManager->setTextSpecialKeyReleased( arg.keysym.sym & ~SDLK_SCANCODE_MASK,
        //                                                    arg.keysym.mod );
        //     }
        // }

        TutorialGameState::keyReleased( arg );
    }
}
