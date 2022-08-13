# ogre-next imgui

#### An imgui backend for ogre-next

![Screenshot](/extra/screenshot.png "Screenshot")

A portable implementation of imgui for projects using ogre-next.

This repo includes an example demo showing how to use it.

### Including in a project
To include imgui in a project, simply include the contents of src/ImguiOgre in your build.
Look through OgreNextImguiGameState.cpp for details of how to use imgui.

### Building the demo
You must have built Ogre ahead of time.
The cmake project expects Ogre to be present in the directory ```Dependencies/```, otherwise they can be specified manually:

```js
cmake -DOGRE_SOURCE=/home/user/build/ogre2/ -DOGRE_BINARIES=/home/user/build/ogre2/build/Debug/ ..
make
```
