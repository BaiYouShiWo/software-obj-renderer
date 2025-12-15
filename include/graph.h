#pragma once
#include <vector>
#include <cstdint>
#include <cassert>
#include <utility>

class Picture {
private:
    std::vector<uint8_t> data_;
    int width_ = 0;
    int height_ = 0;
    int channels_ = 0;
    size_t stride_ = 0;

public:
    Picture() = default;
    
    Picture(int w, int h, int ch) 
        : width_(w), height_(h), channels_(ch),
          stride_(static_cast<size_t>(w) * ch)
    {
        if (w > 0 && h > 0 && ch > 0) {
            data_.resize(height_ * stride_);
        }
    }

    Picture(const Picture& other) = default;
    
    Picture& operator=(const Picture& other) = default;
    
    Picture(Picture&& other) noexcept
        : data_(std::move(other.data_)),
          width_(std::exchange(other.width_, 0)),
          height_(std::exchange(other.height_, 0)),
          channels_(std::exchange(other.channels_, 0)),
          stride_(std::exchange(other.stride_, 0)) {}
    
    Picture& operator=(Picture&& other) noexcept {
        if (this != &other) {
            data_ = std::move(other.data_);
            width_ = std::exchange(other.width_, 0);
            height_ = std::exchange(other.height_, 0);
            channels_ = std::exchange(other.channels_, 0);
            stride_ = std::exchange(other.stride_, 0);
        }
        return *this;
    }

    ~Picture() = default;

    uint8_t& at(int x, int y, int ch) {
        assert(x >= 0 && x < width_ && y >= 0 && y < height_ && ch >= 0 && ch < channels_);
        return data_[y * stride_ + x * channels_ + ch];
    }
    
    const uint8_t& at(int x, int y, int ch) const {
        assert(x >= 0 && x < width_ && y >= 0 && y < height_ && ch >= 0 && ch < channels_);
        return data_[y * stride_ + x * channels_ + ch];
    }

    uint8_t* row_ptr(int y) { 
        assert(y >= 0 && y < height_);
        return data_.data() + y * stride_; 
    }
    
    const uint8_t* row_ptr(int y) const { 
        assert(y >= 0 && y < height_);
        return data_.data() + y * stride_; 
    }

    int width() const { return width_; }
    int height() const { return height_; }
    int channels() const { return channels_; }
    size_t stride() const { return stride_; }
    uint8_t* data() { return data_.data(); }
    const uint8_t* data() const { return data_.data(); }

    void fill(uint8_t value) {
        std::fill(data_.begin(), data_.end(), value);
    }

    void clear() {
        data_.assign(data_.size(), 0);
    }

    uint8_t get_color(int x, int y) const {
        if (x < 0 || x >= width_ || y < 0 || y >= height_ || channels_ < 3) {
            return 0;
        }
        
        uint8_t ri = at(x, y, 0) * 5 / 255;
        uint8_t gi = at(x, y, 1) * 5 / 255;
        uint8_t bi = at(x, y, 2) * 5 / 255;
        return 16 + 36 * ri + 6 * gi + bi;
    }
    
    bool isValid() const {
        return width_ > 0 && height_ > 0 && channels_ > 0 && !data_.empty();
    }
    
    void reset() {
        *this = Picture{};
    }
};