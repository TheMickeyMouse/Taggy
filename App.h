#pragma once
#include "GraphicsDevice.h"
#include "MusicDisplay.h"
#include "Playbar.h"
#include "Sidebar.h"
#include "Titlebar.h"
#include "Track.h"
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
    UIRect uRoot;
    Titlebar uTitlebar;
    Playbar uPlaybar;
    Sidebar uSidebar;
    MusicDisplay uMusic;

    Track track;
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