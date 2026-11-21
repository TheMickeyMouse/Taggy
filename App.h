#pragma once
#include "Cache.h"
#include "Config.h"
#include "GraphicsDevice.h"
#include "MusicDisplay.h"
#include "Playbar.h"
#include "Sidebar.h"
#include "Titlebar.h"
#include "PlaylistView.h"

using namespace Quasi;
using namespace Graphics;
using namespace Math;

class App {
public:
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
    PlaylistView uPlaylistView;

    IndexRef<Track, 0> currentTrackIdx = 0;
    IndexRef<Playlist, 0> currentPlaylistIdx = 0;

    Cache cache;
    Config config;

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

    void SelectTrack(u32 playlistIdx, u32 trackIdx);
    Track& CurrentTrack();

    inline static OptRef<App> Instance = nullptr;
    static Font& GetFont();
    static Font& GetFontBold();
    static ma_engine* GetAudioEngine();

    static Cache& GetCache();
};