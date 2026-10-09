#pragma once
#include "GraphicsDevice.h"
#include "MusicDisplay.h"
#include "Playbar.h"
#include "Sidebar.h"
#include "Titlebar.h"
#include "Track.h"
#include "UIRect.h"
#include "GUI/Canvas.h"
#include "miniaudio.h"

using namespace Quasi;
using namespace Graphics;
using namespace Math;

class App {
    GraphicsDevice gd;
    Canvas canvas;
    iv2 windowSize;
    Font font, fontBold;

    bool debugMode = false;

    // ui stuff
    UIDoc uDoc;
    Titlebar uTitlebar;
    Playbar uPlaybar;
    Sidebar uSidebar;
    MusicDisplay uMusic;

    Track track;

    ma_engine audioEngine;
public:
    App(int w, int h);
    ~App();

    const GLFWwindow* Window() const;
    GLFWwindow* Window();
    float Width() const;
    float Height() const;

    void OnScreenResize();
    void Update();
    bool Run();

    bool IsWindowMinimized() const;

    static String FormatSeconds(int seconds);
};