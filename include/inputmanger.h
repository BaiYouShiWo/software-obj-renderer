#pragma once
#include <Windows.h>
#include <unordered_map>
#include <vector>
class WindowsInputManager {
public:
    struct MouseState {
        float deltaX = 0.0f;
        float deltaY = 0.0f;
        POINT currentPos;
        POINT lastPos;
        bool leftButton = false;
        bool rightButton = false;
        bool middleButton = false;
    };

private:
    HWND windowHandle_;
    MouseState mouseState_;
    std::unordered_map<int, bool> keyStates_;
    std::unordered_map<int, bool> prevKeyStates_;
    bool mouseCaptured_ = false;
    RECT windowRect_;
    
    float mouseSensitivity_ = 0.25f;
    float movementSpeed_ = 5.0f;

public:
    WindowsInputManager(HWND hwnd) : windowHandle_(hwnd) {
        GetClientRect(windowHandle_, &windowRect_);
        resetMousePosition();
    }
    
    // 每帧更新
    void update() {
        updateKeyboardState();
        updateMouseState();
    }
    
    // 鼠标控制
    void captureMouse(bool capture) {
        mouseCaptured_ = capture;
        if (capture) {
            SetCapture(windowHandle_);
            ShowCursor(FALSE);
            resetMousePosition();
        } else {
            ReleaseCapture();
            ShowCursor(TRUE);
        }
    }
    
    bool isKeyPressed(int vkCode) const {
        auto it = keyStates_.find(vkCode);
        auto prevIt = prevKeyStates_.find(vkCode);
        return it != keyStates_.end() && it->second && 
               (prevIt == prevKeyStates_.end() || !prevIt->second);
    }
    
    bool isKeyDown(int vkCode) const {
        auto it = keyStates_.find(vkCode);
        return it != keyStates_.end() && it->second;
    }
    
    MouseState getMouseState() const { return mouseState_; }
    bool isMouseCaptured() const { return mouseCaptured_; }
    float getMouseSensitivity() const { return mouseSensitivity_; }
    void setMouseSensitivity(float sens) { mouseSensitivity_ = sens; }
    float getMovementSpeed() const { return movementSpeed_; }
    void setMovementSpeed(float speed) { movementSpeed_ = speed; }

private:
    void updateKeyboardState() {
        prevKeyStates_ = keyStates_;
        
        int keysToCheck[] = {
            'W', 'S', 'A', 'D', 'Q', 'E', VK_SPACE, VK_SHIFT, VK_CONTROL,
            VK_ESCAPE, VK_UP, VK_DOWN, VK_LEFT, VK_RIGHT, VK_RBUTTON, VK_LBUTTON
        };
        
        for (int vk : keysToCheck) {
            keyStates_[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;
        }
    }
    
    void updateMouseState() {
        mouseState_.lastPos = mouseState_.currentPos;
        
        GetCursorPos(&mouseState_.currentPos);
        ScreenToClient(windowHandle_, &mouseState_.currentPos);

          int screenWidth = GetSystemMetrics(SM_CXSCREEN);
        int screenHeight = GetSystemMetrics(SM_CYSCREEN);

        bool wrapped = false;
        int newX = mouseState_.currentPos.x;
        int newY = mouseState_.currentPos.y;

        if (mouseState_.currentPos.x <= 0) {
            newX = screenWidth - 1;
            wrapped = true;
        } else if (mouseState_.currentPos.x >= screenWidth - 1) {
            newX = 0;
            wrapped = true;
        }

        if (mouseState_.currentPos.y <= 0) {
            newY = screenHeight - 1;
            wrapped = true;
        } else if (mouseState_.currentPos.y >= screenHeight - 1) {
            newY = 0;
            wrapped = true;
        }

        if (wrapped) {
            SetCursorPos(newX, newY);
            mouseState_.lastPos.x = newX;
            mouseState_.lastPos.y = newY;
            mouseState_.currentPos.y = newY;
            mouseState_.currentPos.x = newX;
        
        }
        
        mouseState_.deltaX = static_cast<float>(mouseState_.currentPos.x - mouseState_.lastPos.x);
        mouseState_.deltaY = static_cast<float>(mouseState_.currentPos.y - mouseState_.lastPos.y);
        
        if (mouseCaptured_) {
            resetMousePosition();
            mouseState_.deltaX = 0.0f;
            mouseState_.deltaY = 0.0f;
        }
        
        mouseState_.leftButton = (GetAsyncKeyState(VK_LBUTTON) & 0x8000) != 0;
        mouseState_.rightButton = (GetAsyncKeyState(VK_RBUTTON) & 0x8000) != 0;
        mouseState_.middleButton = (GetAsyncKeyState(VK_MBUTTON) & 0x8000) != 0;
    }
    
    void resetMousePosition() {
        POINT center;
        center.x = windowRect_.left + (windowRect_.right - windowRect_.left) / 2;
        center.y = windowRect_.top + (windowRect_.bottom - windowRect_.top) / 2;
        ClientToScreen(windowHandle_, &center);
        SetCursorPos(center.x, center.y);
        mouseState_.currentPos = center;
    }
};