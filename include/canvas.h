#pragma once
#include <windows.h>
#include <vector>
#include <string>
#include <stdexcept>
#include <format>
#include <sstream> 
#include <string>
#include <iostream>

class Canvas{
private:
    HANDLE console_handle{INVALID_HANDLE_VALUE};
    HANDLE raw_handle{INVALID_HANDLE_VALUE};
    SMALL_RECT srctWriteRect{};
    std::vector<CHAR_INFO> Buffer_CHAR_INFO;
    std::string Buffer_string;
    COORD coordBufSize{};
    COORD coordBufCoord{};
    BOOL fSuccess;

public:
    explicit Canvas(short width, short height) {
        if (width <= 0 || height <= 0) {
            throw std::invalid_argument("Width and Height must be positive values.");
        }
        HANDLE raw_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        console_handle = CreateConsoleScreenBuffer(
       GENERIC_READ |           // read/write access
       GENERIC_WRITE,
       FILE_SHARE_READ |
       FILE_SHARE_WRITE,        // shared
       NULL,                    // default security attributes
       CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE
       NULL);                   // reserved; must be NULL
        if (console_handle == INVALID_HANDLE_VALUE)
        {
            printf("CreateConsoleScreenBuffer failed - (%d)\n", GetLastError());
            throw std::runtime_error("Failed to create console screen buffer.");
        }

        coordBufSize.X = width;
        coordBufSize.Y = height;

        coordBufCoord.X = 0;
        coordBufCoord.Y = 0;
        srctWriteRect = {0, 0, static_cast<SHORT>(coordBufSize.X - 1), static_cast<SHORT>(coordBufSize.Y - 1)};
        Buffer_CHAR_INFO.resize(coordBufSize.X * coordBufSize.Y);

        if (! SetConsoleActiveScreenBuffer(console_handle) )
        {
            printf("SetConsoleActiveScreenBuffer failed - (%d)\n", GetLastError());
        }
    }

    ~Canvas() {
        if (console_handle != INVALID_HANDLE_VALUE){
            CloseHandle(console_handle);
            console_handle = INVALID_HANDLE_VALUE;
        } 
        if (raw_handle != INVALID_HANDLE_VALUE){
            CloseHandle(raw_handle);
            raw_handle = INVALID_HANDLE_VALUE;
        } 
    }   

    Canvas(Canvas&&) = delete;
    Canvas& operator=(Canvas&&) = delete;

    int set_window_size(short width, short height){
        if (width <= 0 || height <= 0) {
            return -1;
        }
        SMALL_RECT windowSize{};
        windowSize.Left = 0;
        windowSize.Top = 0;
        windowSize.Right = width - 1;
        windowSize.Bottom = height - 1;
        if (!SetConsoleWindowInfo(console_handle, TRUE, &windowSize)) {
            printf("SetConsoleWindowInfo failed - (%d)\n", GetLastError());
            return -1;
        }
        return 0;
    }

    auto get_window_size(){
        CONSOLE_SCREEN_BUFFER_INFO csbi{};
        if (!GetConsoleScreenBufferInfo(console_handle, &csbi)) {
            throw std::runtime_error("GetConsoleScreenBufferInfo failed");
        }
        SHORT width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        SHORT height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        return std::make_pair(width, height);
    }



    [[nodiscard]] COORD size() const noexcept { return coordBufSize; }
    [[nodiscard]] int buffer_size() const noexcept { return coordBufSize.X * coordBufSize.Y; }

    int set_frame(const std::vector<CHAR_INFO>& frame){
        if (frame.size() != static_cast<size_t>(buffer_size())) {
            auto msg = std::format("Frame size mismatch: expected {}, got {}", buffer_size(), frame.size());
            throw std::invalid_argument(msg);
        }
        std::swap(Buffer_CHAR_INFO, const_cast<std::vector<CHAR_INFO>&>(frame));
        return 0;
    }

    void update(const std::string& frame){
        fSuccess = WriteConsole(
            console_handle,
            frame.data(),
            static_cast<size_t>(frame.size()),
            NULL,
            NULL);
        if (! fSuccess)
        {
            printf("WriteConsoleOutput failed - (%d)\n", GetLastError());
        }
    }

    void update(){
         auto [w, h] = get_window_size();
         if(w != coordBufSize.X || h != coordBufSize.Y)
        set_window_size(coordBufSize.X, coordBufSize.Y);
        fSuccess = WriteConsoleOutputW(
        console_handle, // screen buffer to write to
        Buffer_CHAR_INFO.data(),        // buffer to copy from
        coordBufSize,     // col-row size of chiBuffer
        coordBufCoord,    // top left src cell in chiBuffer
        &srctWriteRect);  // dest. screen buffer rectangle
        if (! fSuccess)
        {
            printf("WriteConsoleOutput failed - (%d)\n", GetLastError());
        }
    }

};
