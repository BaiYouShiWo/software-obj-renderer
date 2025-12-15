#pragma once
#include <chrono>
#include <thread>

class RenderTimer {
private:
    std::chrono::steady_clock::time_point startTime;
    std::chrono::steady_clock::time_point frameStartTime;
    std::chrono::steady_clock::time_point lastTickTime;
    
    // 修复1：统一使用 duration<double> 或全部转换为相同类型
    std::chrono::duration<double> targetFrameTime;  // 改为 double 精度
    std::chrono::steady_clock::time_point frameEndTargetTime;
    
    float deltaTime = 0.0f;
    float totalTime = 0.0f;
    float fps = 0.0f;
    int frameCount = 0;

public:
    RenderTimer(int targetFPS = 60) {
        setTargetFPS(targetFPS);
        reset();
    }
    
    void setTargetFPS(int fps) {
        if (fps <= 0) fps = 1;
        // 修复2：保持类型一致性，都用 duration<double>
        targetFrameTime = std::chrono::duration<double>(1.0 / fps);
        
        // 修复3：正确计算时间点，需要转换 duration<double> 到具体单位
        auto now = std::chrono::steady_clock::now();
        frameEndTargetTime = now + std::chrono::duration_cast<std::chrono::nanoseconds>(targetFrameTime);
    }
    
    void reset() {
        auto now = std::chrono::steady_clock::now();
        startTime = now;
        frameStartTime = now;
        lastTickTime = now;
        
        // 修复4：reset时也要正确计算
        frameEndTargetTime = now + std::chrono::duration_cast<std::chrono::nanoseconds>(targetFrameTime);
    }
    
    void tick() {
        auto currentTime = std::chrono::steady_clock::now();
        
        auto frameDuration = currentTime - lastTickTime;
        lastTickTime = currentTime;
        
        // 修复5：统一使用 float 转换
        deltaTime = std::chrono::duration<float>(frameDuration).count();
        
        if (deltaTime > 0.8f) {
            deltaTime = 0.8f;
        }
        
        totalTime = std::chrono::duration<float>(currentTime - startTime).count();
        
        frameCount++;
        if (frameCount % 30 == 0) {
            fps = 1.0f / deltaTime;
            std::cout << "FPS: " << fps << ", Delta: " << deltaTime * 1000 << "ms" << std::endl;
        }
    }
    
    void waitIfNeeded() {
        auto currentTime = std::chrono::steady_clock::now();
        
        if (currentTime < frameEndTargetTime) {
            // 修复6：统一duration类型进行比较
            auto waitTime = frameEndTargetTime - currentTime;
            auto threshold = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(2));
            
            if (waitTime > threshold) {
                // 修复7：sleep_for 需要相同单位的duration
                auto sleepTime = waitTime - std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::milliseconds(1));
                std::this_thread::sleep_for(sleepTime);
            }
            
            // 短时间等待：自旋等待
            while (std::chrono::steady_clock::now() < frameEndTargetTime) {
                // 忙等待
            }
        }
        
        // 修复8：正确更新下一帧目标时间
        frameEndTargetTime += std::chrono::duration_cast<std::chrono::nanoseconds>(targetFrameTime);
        
        // 修复9：比较时要统一duration类型
        auto overshoot = currentTime - frameEndTargetTime;
        auto maxDrift = std::chrono::duration_cast<std::chrono::nanoseconds>(targetFrameTime * 3);
        
        if (overshoot > maxDrift) {
            std::cout << "Frame drift detected, skipping ahead" << std::endl;
            frameEndTargetTime = currentTime + std::chrono::duration_cast<std::chrono::nanoseconds>(targetFrameTime);
        }
    }
    float getDeltaTime() const { return deltaTime; }
    float getTotalTime() const { return totalTime; }
    float getFPS() const { return fps; }
    int getTargetFPS() const { return static_cast<int>(1.0f / targetFrameTime.count()); }
};