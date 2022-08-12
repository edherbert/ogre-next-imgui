
#ifndef _Demo_OgreNextImguiGameState_H_
#define _Demo_OgreNextImguiGameState_H_

#include "OgrePrerequisites.h"
#include "TutorialGameState.h"

#include <OgreFrameListener.h>

namespace Demo
{
    class OgreNextImguiGameState : public TutorialGameState, public Ogre::FrameListener
    {
        virtual void generateDebugText( float timeSinceLast, Ogre::String &outText );

    public:
        OgreNextImguiGameState( const Ogre::String &helpDescription );

        virtual void createScene01(void);
        virtual void destroyScene();

        virtual void update( float timeSinceLast );

        virtual bool frameRenderingQueued(const Ogre::FrameEvent& evt);

        virtual void mouseMoved( const SDL_Event &arg );
        virtual void mousePressed( const SDL_MouseButtonEvent &arg, Ogre::uint8 id );
        virtual void mouseReleased( const SDL_MouseButtonEvent &arg, Ogre::uint8 id );
        virtual void textEditing( const SDL_TextEditingEvent &arg );
        virtual void textInput( const SDL_TextInputEvent &arg );
        virtual void keyPressed( const SDL_KeyboardEvent &arg );
        virtual void keyReleased( const SDL_KeyboardEvent &arg );
    };
}

#endif
