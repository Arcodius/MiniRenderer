# MiniRenderer

A light-weight renderer for testing rendering techniques.

## Renderer Features

Implement a 3D scene renderer without relying on open-source rendering tools (e.g., OpenGL). Requirements:

1. **Triangle Mesh Model Loading**  
    - ✅(1) Load triangle mesh models from `.obj` format files.

2. **Camera Model**  
    - ✅(1) Implement perspective projection.  
    - ✅(2) Enable scene navigation (rotation, translation, scaling).  
    - *Optional:*  
      - ⏩(3) Implement orthographic projection.

3. **Rendering**  
    - ✅(1) Include at least two triangle mesh models in the scene.  
    - ✅(2) Implement the Phong lighting model.  
    - (3) Implement a ray tracing algorithm.  
    - (4) Implement texture mapping.  
    - *Optional:*  
      - (1) Implement area light sources.  
      - (2) Implement the BRDF reflection model.  
      - (3) Implement transparent textures.  
      - (4) Implement global illumination.

## CMake usage

### **Build Instructions**

#### **1. Create and Navigate to the Build Directory**  

To initialize the build environment, execute the following commands in your terminal:  

```bash
mkdir -p build && cd ./build
```  

#### **2. Run CMake to Configure the Project**  

CMake will automatically scan your system's development environment and generate the necessary build files. Run the following command:  

```bash
cmake ..
```  

#### **3. Compile the Project**  

Once the configuration is complete, build the project using the following command:  

```bash
cmake --build .
```  

可能的问题

1. glm的点乘、*的混用
2. 向量与mat的乘法顺序和符号
