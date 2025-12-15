#pragma once

#include <memory>
#include <string>

class Picture; 
class RaylibPictureRendererImpl;

class RaylibPictureRenderer {
public:
    RaylibPictureRenderer(int screenWidth, int screenHeight, const std::string& title);
    ~RaylibPictureRenderer();

    RaylibPictureRenderer(const RaylibPictureRenderer&) = delete;
    RaylibPictureRenderer& operator=(const RaylibPictureRenderer&) = delete;

    RaylibPictureRenderer(RaylibPictureRenderer&&) noexcept;
    RaylibPictureRenderer& operator=(RaylibPictureRenderer&&) noexcept;

    bool initialize(const Picture& pic);
    void updateTexture(const Picture& pic);
    void draw();
    bool shouldClose() const;
    void close();
    void setTitle(const std::string& title);

private:
    std::unique_ptr<RaylibPictureRendererImpl> pImpl;
};