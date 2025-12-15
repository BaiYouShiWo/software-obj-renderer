#include "gui.h"
#include "raylib.h"
#include "graph.h"
#include <cmath>
#include <cassert>
#include <string>

class RaylibPictureRendererImpl {
public:
    int screenWidth = 0;
    int screenHeight = 0;
    std::string title;
    Picture localPic;
    ::Texture2D texture = { 0 };
    bool initialized = false;

    void cleanup() {
        if (initialized) {
            if (texture.id != 0) {
                ::UnloadTexture(texture);
                texture = { 0 };
            }
            ::CloseWindow();
            initialized = false;
        }
    }
};

RaylibPictureRenderer::RaylibPictureRenderer(int screenWidth, int screenHeight, const std::string& title)
    : pImpl(std::make_unique<RaylibPictureRendererImpl>()) {
    pImpl->screenWidth = screenWidth;
    pImpl->screenHeight = screenHeight;
    pImpl->title = title;
}

RaylibPictureRenderer::~RaylibPictureRenderer() = default;

RaylibPictureRenderer::RaylibPictureRenderer(RaylibPictureRenderer&&) noexcept = default;
RaylibPictureRenderer& RaylibPictureRenderer::operator=(RaylibPictureRenderer&&) noexcept = default;

bool RaylibPictureRenderer::initialize(const Picture& pic) {
    auto& impl = *pImpl;
    if (impl.initialized) impl.cleanup();

    impl.localPic = pic;

    ::InitWindow(impl.screenWidth, impl.screenHeight, impl.title.c_str());
    ::SetTargetFPS(60);

    if (impl.localPic.channels() != 3 && impl.localPic.channels() != 4) {
        ::TraceLog(::LOG_ERROR, "Only 3 or 4 channel images supported");
        return false;
    }

    ::PixelFormat format = (impl.localPic.channels() == 3)
        ? ::PIXELFORMAT_UNCOMPRESSED_R8G8B8
        : ::PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

    ::Image image = {
        impl.localPic.data(),
        static_cast<int>(impl.localPic.width()),
        static_cast<int>(impl.localPic.height()),
        1,
        format
    };

    impl.texture = ::LoadTextureFromImage(image);
    if (impl.texture.id == 0) {
        ::TraceLog(::LOG_ERROR, "Failed to load texture");
        return false;
    }

    ::SetTextureFilter(impl.texture, ::TEXTURE_FILTER_BILINEAR);
    impl.initialized = true;
    return true;
}

void RaylibPictureRenderer::updateTexture(const Picture& pic) {
    auto& impl = *pImpl;
    if (!impl.initialized) return;

    if (pic.width() != impl.localPic.width() ||
        pic.height() != impl.localPic.height() ||
        pic.channels() != impl.localPic.channels()) {
        initialize(pic);
        return;
    }

    impl.localPic = pic;
    ::UpdateTexture(impl.texture, impl.localPic.data());
}

void RaylibPictureRenderer::draw() {
    auto& impl = *pImpl;
    if (!impl.initialized) return;

    ::BeginDrawing();
    ::ClearBackground(::BLACK);

    float scale = std::fminf(
        static_cast<float>(impl.screenWidth) / impl.localPic.width(),
        static_cast<float>(impl.screenHeight) / impl.localPic.height()
    );
    int renderWidth = static_cast<int>(impl.localPic.width() * scale);
    int renderHeight = static_cast<int>(impl.localPic.height() * scale);
    int posX = (impl.screenWidth - renderWidth) / 2;
    int posY = (impl.screenHeight - renderHeight) / 2;

    ::DrawTexturePro(
        impl.texture,
        { 0.0f, 0.0f, static_cast<float>(impl.texture.width), static_cast<float>(impl.texture.height) },
        { static_cast<float>(posX), static_cast<float>(posY), static_cast<float>(renderWidth), static_cast<float>(renderHeight) },
        { 0.0f, 0.0f }, 0.0f, ::WHITE
    );

    ::DrawText("Press ESC to exit", 10, 10, 20, ::GRAY);
    ::EndDrawing();
}

bool RaylibPictureRenderer::shouldClose() const {
    return ::WindowShouldClose();
}

void RaylibPictureRenderer::close() {
    if (pImpl) pImpl->cleanup();
}

void RaylibPictureRenderer::setTitle(const std::string& title) {
    auto& impl = *pImpl;
    impl.title = title;
    if (impl.initialized) {
        ::SetWindowTitle(impl.title.c_str());
    }
}