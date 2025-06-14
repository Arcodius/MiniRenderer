#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <string>
#include "Renderer.h"
#include "Scene.h"

void setupScene(Scene& scene) {
    // 地面
    Material groundMaterial;
    groundMaterial.diffuseColor = glm::vec3(0.5f, 0.5f, 0.5f); // 灰色
    groundMaterial.specularColor = glm::vec3(0.1f, 0.1f, 0.1f); // 微弱高光
    groundMaterial.roughness = 0.8f; // 粗糙
    groundMaterial.metallic = 0.0f; // 非金属

    std::shared_ptr<Plane> ground = std::make_shared<Plane>(
        true,
        glm::vec3(0.0f, -1.0f, 0.0f), // 地面位置
        glm::vec3(0.0f, 1.0f, 0.0f), // 法线方向
        groundMaterial
    );
    ground->setScale(glm::vec3(5.f, 1.0f, 5.f));
    scene.addObject(ground);

    // 球体
    Material sphereMaterial;
    sphereMaterial.diffuseColor = glm::vec3(1.0f, 0.0f, 0.0f); // 红色
    sphereMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f); // 白色高光
    sphereMaterial.roughness = 0.2f; // 光滑
    sphereMaterial.metallic = 0.0f; // 非金属

    std::shared_ptr<Sphere> sphere = std::make_shared<Sphere>(
        glm::vec3(2.0f, 0.0f, 0.0f), // 球体位置
        0.5f, // 半径
        sphereMaterial
    );
    scene.addObject(sphere);

    // 立方体
    Material cubeMaterial;
    cubeMaterial.diffuseColor = glm::vec3(0.0f, 0.0f, 1.0f); // 蓝色
    cubeMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f); // 白色高光
    cubeMaterial.roughness = 0.3f; // 光滑
    cubeMaterial.metallic = 0.0f; // 非金属

    std::shared_ptr<Cube> cube = std::make_shared<Cube>(
        glm::vec3(-2.0f, 0.0f, 0.0f), // 立方体位置
        glm::vec3(1.0f), // 尺寸
        cubeMaterial
    );
    scene.addObject(cube);

    // 圆柱
    Material cylinderMaterial;
    cylinderMaterial.diffuseColor = glm::vec3(0.0f, 1.0f, 0.0f); // 绿色
    cylinderMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f); // 白色高光
    cylinderMaterial.roughness = 0.4f; // 光滑
    cylinderMaterial.metallic = 0.0f; // 非金属

    std::shared_ptr<Cylinder> cylinder = std::make_shared<Cylinder>(
        glm::vec3(0.0f, 0.0f, 2.0f), // 圆柱位置
        0.5f, // 半径
        1.5f, // 高度
        cylinderMaterial
    );
    scene.addObject(cylinder);

    // 圆锥
    Material coneMaterial;
    coneMaterial.diffuseColor = glm::vec3(1.0f, 1.0f, 0.0f); // 黄色
    coneMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f); // 白色高光
    coneMaterial.roughness = 0.5f; // 光滑
    coneMaterial.metallic = 0.0f; // 非金属

    std::shared_ptr<Cone> cone = std::make_shared<Cone>(
        glm::vec3(0.0f, 0.0f, -2.0f), // 圆锥位置
        0.5f, // 半径
        1.5f, // 高度
        coneMaterial
    );
    scene.addObject(cone);

    // 环面
    Material torusMaterial;
    torusMaterial.diffuseColor = glm::vec3(1.0f, 0.5f, 0.0f); // 橙色
    torusMaterial.specularColor = glm::vec3(1.0f, 1.0f, 1.0f); // 白色高光
    torusMaterial.roughness = 0.3f; // 光滑
    torusMaterial.metallic = 0.0f; // 非金属

    std::shared_ptr<Torus> torus = std::make_shared<Torus>(
        glm::vec3(0.0f, 0.0f, 0.0f), // 环面位置
        1.0f, // 主半径
        0.3f, // 次半径
        torusMaterial
    );
    scene.addObject(torus);

    // 灯光
    std::shared_ptr<PointLight> light = std::make_shared<PointLight>(
        glm::vec3(1.0f, 1.0f, 1.0f), // 灯光颜色
        5.0f, // 强度
        glm::vec3(0.0f, 2.0f, 0.0f), // 灯光位置
        100.0f // 距离衰减
    );
    scene.addLight(light);

    // 设置相机
    scene.camera.setPerspective(true);
    scene.camera.setFovY(90.0f);
    scene.camera.setPosition(glm::vec3(0.0f, 2.0f, 5.0f));
    scene.camera.setTarget(glm::vec3(0.0f, 0.0f, 0.0f));
}


int main(int argc, char* argv[])
{
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);  // output all logs

    // initialize
    if ((int)SDL_Init(SDL_INIT_VIDEO) < 0){
        SDL_Log("SDL_Init failed: $s", SDL_GetError());
    }

    const int width = 160, height = 90;
    // Create a window
    SDL_Window* window = SDL_CreateWindow("MiniRenderer", width, height, SDL_WINDOW_RESIZABLE);
    if (!window){
        SDL_Log("Could not create a window: $s", SDL_GetError());
        return -1;
    }
    SDL_SetWindowPosition(window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
    // Create renderer connected to the window
    SDL_Renderer* sdlRenderer = SDL_CreateRenderer(window, nullptr);
    if (!sdlRenderer){
        SDL_Log("Could not create renderer: $s", SDL_GetError());
        return -1;
    }
    // Create a streaming texture
    SDL_Texture* texture = SDL_CreateTexture(
        sdlRenderer,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_STREAMING,
        width, height
    );
    if (!texture) {
        SDL_Log("Could not create texture: %s", SDL_GetError());
        return -1;
    }
    Scene scene = Scene();
    setupScene(scene);
    Renderer renderer(width, height);

    // Event loop
    SDL_Event event{};
    bool keep_going = true;
    bool mouseRightButtonDown = false;
    bool useRayTracing = false; // 是否使用光线追踪渲染
    bool justEnteredRelativeMode = false; // first frame protection
    const bool* keyboardState = SDL_GetKeyboardState(NULL); // 监控keyboard状态
    
    Uint64 lastTick = SDL_GetPerformanceCounter();
    float deltaTime = 0.016f;
    while(keep_going){
        // 在主循环内部，渲染循环的开始处
        // Uint64 currentTick = SDL_GetPerformanceCounter();
        // deltaTime = (float)(currentTick - lastTick) / (float)SDL_GetPerformanceFrequency();
        // lastTick = currentTick;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT ||
            (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_ESCAPE)) {
                keep_going = false;
            }

            // 切换渲染模式
            if (event.type == SDL_EVENT_KEY_DOWN && event.key.key == SDLK_TAB) {
                useRayTracing = !useRayTracing; // Toggle rendering mode
                if (useRayTracing) {
                    SDL_Log("Switched to Ray Tracing mode");
                } else {
                    SDL_Log("Switched to Rasterization mode");
                }
            }

            // 右键按下启用相对鼠标模式
            if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN && event.button.button == SDL_BUTTON_RIGHT) {
                mouseRightButtonDown = true;
                justEnteredRelativeMode = true;

                SDL_SetWindowMouseGrab(window, true);
                SDL_SetWindowRelativeMouseMode(window, true);

                float _, __;
                SDL_GetRelativeMouseState(&_, &__); // clear remaining delta
            }
            // 右键抬起取消相对鼠标模式
            else if (event.type == SDL_EVENT_MOUSE_BUTTON_UP && event.button.button == SDL_BUTTON_RIGHT) {
                mouseRightButtonDown = false;
                SDL_SetWindowMouseGrab(window, false);
                SDL_SetWindowRelativeMouseMode(window, false);
            }
            else if (event.type == SDL_EVENT_MOUSE_WHEEL) {
                float scrollY = static_cast<float>(event.wheel.y); // 正数为向上滚动，负数为向下
                scene.camera.handleScroll(scrollY, deltaTime);
            }
        }

        if (mouseRightButtonDown) {
            float dx, dy;
            SDL_GetRelativeMouseState(&dx, &dy);

            if (!justEnteredRelativeMode) {
                scene.camera.processMouseMotion(dx, dy);
            } else {
                justEnteredRelativeMode = false; // 跳过第一帧
            }
        }

        if (mouseRightButtonDown) { // 只有右键按下时才处理相机移动
            if (keyboardState[SDL_SCANCODE_W]) {
                scene.camera.handleKeyPress(SDLK_W, deltaTime);
            }
            if (keyboardState[SDL_SCANCODE_S]) {
                scene.camera.handleKeyPress(SDLK_S, deltaTime);
            }
            if (keyboardState[SDL_SCANCODE_A]) {
                scene.camera.handleKeyPress(SDLK_A, deltaTime);
            }
            if (keyboardState[SDL_SCANCODE_D]) {
                scene.camera.handleKeyPress(SDLK_D, deltaTime);
            }
            // 添加其他你需要的按键处理
        }

        

        // rendering part
        if (useRayTracing) {
            renderer.renderRayTracing(scene);
        } else {
            renderer.render(scene);
        }
        // update framebuffer to SDL_Texture
        SDL_UpdateTexture(texture, nullptr, renderer.framebuffer.data(), width * sizeof(uint32_t));

        int window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);

        // keep original aspect ratio and keep the image centered
        float aspect_ratio = static_cast<float>(width) / height;
        int draw_width = window_width;
        int draw_height = static_cast<int>(window_width / aspect_ratio);
        if (draw_height > window_height) {
            draw_height = window_height;
            draw_width = static_cast<int>(window_height * aspect_ratio);
        }

        SDL_FRect dst_rect;
        dst_rect.w = static_cast<float>(draw_width);
        dst_rect.h = static_cast<float>(draw_height);
        dst_rect.x = (window_width - dst_rect.w) / 2.0f;
        dst_rect.y = (window_height - dst_rect.h) / 2.0f;

        // Draw the texture to the screen
        SDL_RenderClear(sdlRenderer);
        SDL_RenderTexture(sdlRenderer, texture, nullptr, &dst_rect); // adapt to window size
        SDL_RenderPresent(sdlRenderer);

        SDL_Delay(16); // confine to 60fps
    }

    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

