#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_sdlrenderer3.h>

#include <string>

#include "Renderer.h"
#include "ResourceManager.h"
#include "Scene.h"

int main(int argc, char* argv[])
{
    SDL_SetLogPriorities(SDL_LOG_PRIORITY_VERBOSE);  // output all logs

    // initialize
    if ((int)SDL_Init(SDL_INIT_VIDEO) < 0){
        SDL_Log("SDL_Init failed: $s", SDL_GetError());
    }

    const int width = 160, height = 90;
    const int screenWidth = 1920, screenHeight = 1080;
    // Create a window
    SDL_Window* window = SDL_CreateWindow("MiniRenderer", screenWidth / 2, screenHeight / 2, SDL_WINDOW_RESIZABLE);
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
    // Scene initialization
    Scene scene = Scene();
    Renderer renderer(width, height);

    // 初始化 ImGui 上下文
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;

    // 设置样式
    ImGui::StyleColorsDark(); // 或 ImGui::StyleColorsClassic()

    // 初始化 ImGui 的 SDL 和 SDL_Renderer 后端
    ImGui_ImplSDL3_InitForSDLRenderer(window, sdlRenderer);
    ImGui_ImplSDLRenderer3_Init(sdlRenderer);


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
        }

        

        // 1. 渲染到 framebuffer
        if (useRayTracing) {
            renderer.renderRayTracing(scene);
        } else {
            renderer.render(scene);
        }

        // 2. 上传 framebuffer 到 SDL_Texture
        SDL_UpdateTexture(texture, nullptr, renderer.framebuffer.data(), width * sizeof(uint32_t));

        // 3. 计算目标矩形
        int window_width, window_height;
        SDL_GetWindowSize(window, &window_width, &window_height);
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

        // 4. 清除屏幕
        SDL_RenderClear(sdlRenderer);

        // 5. 先画 framebuffer 到窗口（非常重要）
        SDL_RenderTexture(sdlRenderer, texture, nullptr, &dst_rect);

        // 6. 再开始 ImGui 渲染帧
        ImGui_ImplSDL3_NewFrame();
        ImGui_ImplSDLRenderer3_NewFrame();
        ImGui::NewFrame();

        // 7. 画 UI
        {
            // 设置 UI 窗口位置和大小
            ImGui::SetNextWindowPos(ImVec2(0, 0)); // 窗口顶端
            ImGui::SetNextWindowSize(ImVec2(static_cast<float>(window_width), 20)); // 宽度为窗口宽度，高度为 100

            ImGui::Begin("Menu", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);

            // Render Frame 按钮居中
            ImGui::SetCursorPosX((ImGui::GetWindowSize().x - ImGui::CalcTextSize("Render Frame").x) / 2.0f);
            if (ImGui::Button("Render Frame")) {
                if (!useRayTracing) { // not rendered
                    printf("Begin ray tracing...\n");
                    renderer.clearBuffers();
                    renderer.renderRayTracing(scene);
                    printf("Ray tracing finished.\n");
                }
                const Buffer<uint32_t>& buffer = renderer.getBuffer();
                ResourceManager::saveFramebufferToBMP("ray_tracing_output.bmp", renderer.getBuffer());
            }

            // 相机位置显示在右上角
            ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 200, 10)); // 偏移到右上角
            ImGui::Text("Camera Pos: (%.2f, %.2f, %.2f)", 
                scene.camera.getPosition().x, scene.camera.getPosition().y, scene.camera.getPosition().z);

            ImGui::End();
        }

        // 8. 渲染 ImGui 到 SDL
        ImGui::Render();
        ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), sdlRenderer);

        // 9. 提交本帧
        SDL_RenderPresent(sdlRenderer);
        SDL_Delay(16); // confine to 60fps
    }

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    SDL_DestroyRenderer(sdlRenderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

