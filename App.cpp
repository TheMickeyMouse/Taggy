#include "App.h"

#include <thread>

#include "glp.h"
#include "WinUtils.h"
#include "Utils/Text/UTF.h"

App::App(int w, int h)
    : gd(GraphicsDevice::Initialize({ w, h }, { .beginPosition = { 300 }, .windowTitle = "Taggy" })),
      canvas(gd), windowSize { w, h },

      font(Font::LoadFile("C:/Users/User/AppData/Local/Microsoft/Windows/Fonts/JetBrainsMono-Regular.ttf", 48.0f)),
      fontBold(Font::LoadFile("C:/Users/User/AppData/Local/Microsoft/Windows/Fonts/JetBrainsMono-Bold.ttf", 48.0f)),

      uDoc({ 0, (fv2)windowSize }),
      uTitlebar(uDoc.Root()),
      uPlaybar (uDoc.Root(), canvas),
      uSidebar (uDoc.Root(), canvas),
      uMusic   (uDoc.Root()),
      uPlaylistView(uDoc.Root(), canvas)
{
    Instance = *this;

    glfwSetWindowSizeLimits(gd.GetWindow(), 800, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
    WinUtils::SetHwnd(gd.GetWindow());
    WinUtils::UseCustomTitlebar(Titlebar::HEIGHT, Titlebar::HEIGHT);
    canvas.FlipYDirection();

    if (const auto result = ma_engine_init(nullptr, &audioEngine); result != MA_SUCCESS) {
        Debug::QCritical$("Couldn't initialize audio engine! err code: {}", (int)result);
    }

    config = Config::Read();
    cache = Cache::Read();

    uPlaylistView.Select(config.playlistLibrary, 0);
    Track::Load(CurrentTrack(), false);

    // uDoc.PrettyPrintStructure();
}

App::~App() {
    ma_engine_uninit(&audioEngine);

    config.Write();
    cache.Write();
}

const GLFWwindow* App::Window() const {
    return gd.GetWindow();
}
GLFWwindow* App::Window() {
    return gd.GetWindow();
}

float App::Width() const {
    return (float)windowSize.x;
}

float App::Height() const {
    return (float)windowSize.y;
}

void App::OnScreenResize() {
    windowSize = gd.GetWindowSize();
    canvas.SetViewport({ { 0, (float)windowSize.y }, { (float)windowSize.x, 0 } });
    canvas.SetCanvasSize(windowSize);

    // ui handling
    uDoc.SetRect({ 0, (fv2)windowSize });
    uDoc.ComputeLayout();
    uPlaylistView.OnLayoutChange();
}

void App::Update() {
    if (gd.GetWindowSize() != windowSize && gd.GetWindowPos().AllGreater(-16000)) { // resize happened, dont count minimized states
        OnScreenResize();
    }
}

bool App::Run() {
    Update();

    const float dt = std::min(gd.GetIO().DeltaTime(), 0.333f);
    gd.Begin();

    if (glfwGetWindowAttrib(gd.GetWindow(), GLFW_FOCUSED)) {
        gd.PollEvents();

        uPlaylistView.Update(dt);
        uSidebar.Update(dt);
    }

    if (!IsWindowMinimized() && gd.GetWindowPos().AllGreater(-16000)) {
        canvas.BeginFrame();

        const auto& io = gd.GetIO();

        uDoc.CheckMouseStates(io.GetMousePos(), io.LeftMouse().Pressed(), io.LeftMouse().OnPress(), io.GetMouseScrollDelta());

        if (io['K'].OnPress() && io.Shift()) {
            return false;
        }

        canvas.NoStroke();
        canvas.Fill(BACKGROUND);
        canvas.DrawRect({ 0, { Width(), Height() } });

        auto& currentTrack = CurrentTrack();
        uTitlebar    .Draw(canvas);
        uMusic       .Draw(canvas, currentTrack);
        uPlaybar     .Draw(canvas, currentTrack, io);
        uPlaylistView.Draw(canvas);
        uSidebar     .Draw(canvas);

        debugMode ^= io["I"].OnPress() && io.Shift();
        if (debugMode) {
            canvas.Stroke(1);
            canvas.DrawText(
                Text::Format(
                    "WPos = {}, {}x{}; "
                    "LClick = {}; "
                    "Mouse = {}; "
                    "MouseDown = {}; "
                    "Hovering = '{}'\n"
                    "FPS = {}; "
                    "FrameTime = {}.{:03}ms; ",
                    gd.GetWindowPos(), gd.GetWindowSize().x, gd.GetWindowSize().y,
                    io.LeftMouse().ClickedPos(),
                    io.GetMousePos(),
                    io.LeftMouse().Pressed(),
                    uDoc.DebugNameOf(uDoc.hoveredID),
                    std::llround(io.Framerate()),
                    gd.GetFrameTime().count() / 1000'000, (gd.GetFrameTime().count() / 1000) % 1000
                ),
                11.0f,
                { 0, Height() },
                { .alignment = TextAlign::LEFT | TextAlign::VBOTTOM }
            );

            uDoc.DrawDebug(canvas);
        }

        if (io[IO::Key::P].OnPress() && io.Shift()) {
            uDoc.PrettyPrintStructure();
        }

        if (io[IO::Key::F].OnPress()) {
            for (auto& f : WinUtils::OpenFileDialog()) {
                Debug::QInfo$("opened file {}", Text::Utf16To8(f.c_str()));
            }
        }

        if (io[IO::Key::SPACE].OnPress()) {
            CurrentTrack().Toggle();
        }

        canvas.EndFrame();
    } else {
        gd.MarkNothingRendered();
    }

    gd.End();

    return gd.WindowIsOpen();
}

bool App::IsWindowMinimized() const {
    return glfwGetWindowAttrib(gd.GetMainWindow(), GLFW_ICONIFIED);
}

String App::FormatSeconds(int seconds) {
    return Text::Format("{}:{:02}", seconds / 60, seconds % 60);
}

void App::SelectTrack(u32 playlistIdx, u32 trackIdx) {
    if (currentPlaylistIdx == playlistIdx && currentTrackIdx == trackIdx) return;

    CurrentTrack().Stop();
    currentPlaylistIdx = playlistIdx;
    currentTrackIdx    = trackIdx;
    CurrentTrack().Start();
}

Track& App::CurrentTrack() {
    return config.playlistLibrary.playlists[currentPlaylistIdx].tracks[currentTrackIdx];
}

Font& App::GetFont() {
    return Instance->font;
}

Font& App::GetFontBold() {
    return Instance->fontBold;
}

ma_engine* App::GetAudioEngine() {
    return &Instance->audioEngine;
}

Cache& App::GetCache() {
    return Instance->cache;
}
