#pragma once
#include "GraphicsDevice.h"
#include "Icon.h"
#include "Sidebar.h"
#include "Titlebar.h"
#include "UIRect.h"
#include "GUI/Canvas.h"

using namespace Quasi;
using namespace Graphics;
using namespace Math;

class App {
    GraphicsDevice gd;
    Canvas canvas;
    Font font;
    iv2 windowSize;
    bool debugMode = false;

    // ui stuff
    UIRect root;
    Titlebar titlebar;
    Sidebar sidebar;

    static constexpr float TITLEBAR_HEIGHT = 40.0f, SIDEBAR_WIDTH = 80.0f;
public:
    App(int w, int h);

    const GLFWwindow* Window() const;
    GLFWwindow* Window();
    float Width() const;
    float Height() const;

    void OnScreenResize();
    void Update();
    bool Run();
};