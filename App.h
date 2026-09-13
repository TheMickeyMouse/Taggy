#pragma once
#include "GraphicsDevice.h"
#include "GUI/Canvas.h"

using namespace Quasi;
using namespace Graphics;
using namespace Math;

class App {
    GraphicsDevice gd;
    Canvas canvas;
    iv2 windowSize;
    bool debugMode;

    static constexpr float TITLEBAR_HEIGHT = 50.0f;
public:
    App(int w, int h, bool debug);

    const GLFWwindow* Window() const;
    GLFWwindow* Window();
    float Width() const;
    float Height() const;

    void DrawWindowBtns();
    // 0 = close, 1 = fullscren, 2 = minimize
    void DrawCloseBtn(int btnNum);

    void Update();
    bool Run();
};