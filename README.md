# ogre-next imgui

#### A Dear ImGui Backend for ogre-next

![Screenshot](/extra/screenshot.png "Screenshot")

A portable backend of [imgui](https://github.com/ocornut/imgui/) for projects using [ogre-next](https://github.com/OGRECave/ogre-next).

This project works with ogre-next v2.3 and has been shown to work on all Ogre's render systems

 * OpenGL3+
 * Vulkan
 * Metal
 * D3D11

This repo includes an example demo showing how to use the backend.

### Including in a Project
To include imgui in a project, simply include the contents of ```src/ImguiOgre``` in your build.
Look through ```src/OgreNextImguiGameState.cpp``` for details of how to use the project in your codebase.

### Building the Demo

Clone with submodules to ensure you get the imgui dependency:
```bash
git clone --recurse-submodules --shallow-submodules https://github.com/edherbert/ogre-next-imgui.git
```

#### Linux
You must have built Ogre ahead of time.
The cmake project expects Ogre to be present in the directory ```Dependencies/```, otherwise they can be specified manually:

```bash
cmake -DOGRE_SOURCE=/home/user/build/ogre2/ -DOGRE_BINARIES=/home/user/build/ogre2/build/Debug/ ..
make
```

Note: If you've only built Ogre as Debug, ensure you specify a cmake Debug build so it can correctly find the libraries.
```bash
cmake -DOGRE_SOURCE=/home/user/build/ogre2/ -DOGRE_BINARIES=/home/user/build/ogre2/build/Debug/ -DCMAKE_BUILD_TYPE=Debug ..
```

#### MacOS
This project has only been tested with Xcode builds.
You are expected to have built Ogre as static libraries, using the flags

```bash
-DOGRE_STATIC=TRUE -DOGRE_BUILD_LIBS_AS_FRAMEWORKS=FALSE
```

Ensure you build for xcode
```bash
cmake -DOGRE_SOURCE=/Users/user/build/ogre2/ -DOGRE_BINARIES=/Users/user/build/ogre2/build/Debug/ -GXcode ..
```

### Special thanks
This project contains code taken from a number of sources.
I have cleaned it up and added the features I thought were necessary to make it easily useable.

Original implementation: [Crashy](https://forums.ogre3d.org/viewtopic.php?t=89081)

Added Metal support: [Me](https://forums.ogre3d.org/viewtopic.php?t=94958)

General fixes: [Various people](https://forums.ogre3d.org/viewtopic.php?t=93889)

Updated to Ogre2.3: [Vian](https://forums.ogre3d.org/viewtopic.php?t=96798)

This imgui backend was floating around in bits and pieces on the Ogre forum, I've simply unified them somwhere.
Thanks to all these people :)