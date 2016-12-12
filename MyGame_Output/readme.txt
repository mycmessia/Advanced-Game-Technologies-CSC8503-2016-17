**********************
*** FILE STRUCTURE ***
**********************

The file structure has been rearranged in order to make it easier to
see what elements will be shipped with the game and which don't. 

This folder is always set as the executables working directory when
compiled via visual studio, meaning all Data/* elements are accessed
program side by the filepath "Data/Meshes/mymesh.obj" etc..

This means that to publish your game, you just have to copy the
'../Build/Release/[mygame].exe" file here and everything will run
as is without visual studio.