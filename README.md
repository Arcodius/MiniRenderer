# MiniRenderer

A light-weight renderer for testing rendering techniques.

## Renderer Features

Implement a 3D scene renderer without relying on open-source rendering tools (e.g., OpenGL). Requirements:

1. **Triangle Mesh Model Loading**  
    - (1) Load triangle mesh models from `.obj` format files.

2. **Camera Model**  
    - (1) Implement perspective projection.  
    - (2) Enable scene navigation (rotation, translation, scaling).  
    - *Optional:*  
      - (3) Implement orthographic projection.

3. **Rendering**  
    - (1) Include at least two triangle mesh models in the scene.  
    - (2) Implement the Phong lighting model.  
    - (3) Implement a ray tracing algorithm.  
    - (4) Implement texture mapping.  
    - *Optional:*  
      - (1) Implement area light sources.  
      - (2) Implement the BRDF reflection model.  
      - (3) Implement transparent textures.  
      - (4) Implement global illumination.

## CMake usage

1. Create and change to build dir:
mkdir build
cd ./build
2. run cmake command to init project config, and cmake would automatically scan the dev env of system:
cmake ..
3. cmake command to compile the project:
cmkae --build .
