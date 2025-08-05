# MiniRenderer

A light-weight renderer for testing rendering techniques with optional CUDA acceleration.

## Renderer Features

Implement a 3D scene renderer without relying on open-source rendering tools (e.g., OpenGL). Requirements:

1. **Triangle Mesh Model Loading**  
    - ✅(1) Load triangle mesh models from `.obj` format files.

2. **Camera Model**  
    - ✅(1) Implement perspective projection.  
    - ✅(2) Enable scene navigation (rotation, translation, scaling).  
    - *Optional:*  
      - ✅(3) Implement orthographic projection.

3. **Rendering**  
    - ✅(1) Include at least two triangle mesh models in the scene.  
    - ✅(2) Implement the Phong lighting model.  
    - ✅(3) Implement a ray tracing algorithm.  
    - ✅(4) Implement texture mapping.  
    - *Optional:*  
      - ✅(1) Implement area light sources.  
      - ✅(2) Implement the BRDF reflection model.  
      - (3) Implement transparent textures.  
      - (4) Implement global illumination.

4. **CUDA Acceleration** ✨ **NEW!**
    - ✅(1) GPU-accelerated ray tracing using CUDA
    - ✅(2) Runtime toggle between CPU and GPU rendering
    - (3) GPU-accelerated rasterization (planned)

## CUDA Acceleration

This renderer now supports CUDA acceleration for significantly improved performance, especially for ray tracing operations.

### **Prerequisites**

- NVIDIA GPU with CUDA Compute Capability 7.5 or higher
- CUDA Toolkit 11.0 or later
- Compatible NVIDIA graphics drivers

### **Features**

- **GPU Ray Tracing**: Accelerated ray-triangle intersection and shading calculations
- **Runtime Toggle**: Switch between CPU and GPU rendering during runtime via the UI
- **Anti-aliasing**: Multi-sample anti-aliasing (MSAA) for smoother image quality
- **Automatic Fallback**: Gracefully falls back to CPU rendering if CUDA is not available

### **Usage**

1. Ensure CUDA is properly installed on your system
2. Build the project with CUDA support (automatic if CUDA is detected)
3. Run the application
4. Use the "CUDA Acceleration" checkbox in the UI to toggle GPU acceleration
5. Ray tracing will automatically use GPU acceleration when enabled

### **Performance Notes**

- GPU acceleration provides 10-50x speedup for ray tracing workloads
- Performance improvement depends on scene complexity and GPU specifications
- For simple scenes, CPU rendering may be sufficient

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
