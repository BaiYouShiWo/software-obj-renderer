#pragma once
#include "linear.h"
#include "inputmanger.h"
class Camera {
private:
    float fov_;
    float aspect_ratio_;
    float near_clip_;
    float far_clip_;

    float yaw = -90.f;
    float pitch = 0.f;
    float movementSpeed = 5.f;
    float mouseSensitivity = 0.01f;

    vec3 position;
    vec3 front;
    vec3 up;
    vec3 right;

    Matrix ViewMatrix_{4,4};
    Matrix PerspectiveMatrix_{4,4};

public:

    explicit Camera(float fov, float aspect_ratio, float near_clip, float far_clip,
                    vec3 pos = vec3(0,0,4), vec3 target = vec3(0,0,0), vec3 up = vec3(0,1,0))
        : fov_(fov), aspect_ratio_(aspect_ratio), near_clip_(near_clip), far_clip_(far_clip),
          position(pos), up(up) ,front((target - pos).normalize()), right(front.cross(up).normalize())
          {
            ViewMatrix_ = getViewMatrix(pos, target, up);
            PerspectiveMatrix_ = getPerspectiveMatrix(fov_, aspect_ratio_, near_clip_, far_clip_);
        }
    
    void update(WindowsInputManager& input, float deltaTime) {
        handleMouseInput(input, deltaTime);
        handleKeyboardInput(input, deltaTime);
        updateViewMatrix(position, position + front, up);
    }

private:
    void updateViewMatrix(vec3 pos, vec3 target, vec3 up) {
        ViewMatrix_ = getViewMatrix(pos, target, up);
    }

    void updatePerspectiveMatrix(float fov, float aspect_ratio, float near_clip, float far_clip) {
        PerspectiveMatrix_ = getPerspectiveMatrix(fov, aspect_ratio, near_clip, far_clip);
    }

    void handleMouseInput(WindowsInputManager& input, float deltaTime) {
        auto mouse = input.getMouseState();
        
            yaw += mouse.deltaX * deltaTime * 2;
            pitch -= mouse.deltaY * deltaTime * 2;
            
            pitch = std::clamp(pitch, -89.0f, 89.0f);
            
            updateCameraVectors();
        
    }
    
    void handleKeyboardInput(WindowsInputManager& input, float deltaTime) {
        float velocityMultiplier = 1.0f;
        if (input.isKeyDown(VK_SHIFT)) velocityMultiplier = 2.0f; // 冲刺
        
        
        float currentSpeed = movementSpeed * velocityMultiplier * deltaTime;
        
        if (input.isKeyDown('W')) position =  position + front * currentSpeed;
        if (input.isKeyDown('S')) position = position - front * currentSpeed;
        if (input.isKeyDown('A')) position = position - right * currentSpeed;
        if (input.isKeyDown('D')) position = position + right * currentSpeed;
        if (input.isKeyDown(VK_CONTROL)) position = position - up * currentSpeed;
        if (input.isKeyDown(VK_SPACE)) position = position + up * currentSpeed;

        if (input.isKeyPressed('R')) {
            position = vec3(0, 0, 4);
            yaw = -90.0f;
            pitch = 0.0f;
            updateCameraVectors();
        }
    }

    void updateCameraVectors() {
        vec3 f;
        f.x = cosf(yaw * (float)PI / 180.f) * cosf(pitch * (float)PI / 180.f);
        f.y = sinf(pitch * (float)PI / 180.f);
        f.z = sinf(yaw * (float)PI / 180.f) * cosf(pitch * (float)PI / 180.f);
        front = f.normalize();
        right = front.cross(vec3(0,1,0)).normalize();
        up = right.cross(front).normalize();
    }

public:
    [[nodiscard]] float fov() const { return fov_; }
    [[nodiscard]] float aspect_ratio() const { return aspect_ratio_; }
    [[nodiscard]] float near_clip() const { return near_clip_; }
    [[nodiscard]] float far_clip() const { return far_clip_; }
    [[nodiscard]] const Matrix& view_matrix() const { return ViewMatrix_; }
    [[nodiscard]] const Matrix& perspective_matrix() const { return PerspectiveMatrix_; }
};