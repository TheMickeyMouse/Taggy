#include "App.h"

#include <thread>

#include "glp.h"
#include "WinUtils.h"

App::App(int w, int h)
    : gd(GraphicsDevice::Initialize({ w, h }, { .beginPosition = { 300 }, .windowTitle = "Taggy" })),
      canvas(gd), windowSize { w, h },

      uRoot(UIRect::Root({ 0, (fv2)windowSize })),
      uTitlebar(uRoot),
      uPlaybar(uRoot, canvas),
      uSidebar(uRoot, canvas),
      uMusic(uRoot)
{
    glfwSetWindowSizeLimits(gd.GetWindow(), 800, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
    WinUtils::UseCustomTitlebar(gd.GetWindow(), Titlebar::HEIGHT, Titlebar::HEIGHT);
    canvas.FlipYDirection();
    canvas.SetFont(font = Font::LoadFile("C:/Users/User/AppData/Local/Microsoft/Windows/Fonts/JetBrainsMono-Bold.ttf", 48.0f));

    if (const auto result = ma_engine_init(nullptr, &audioEngine); result != MA_SUCCESS) {
        Debug::QCritical$("Couldn't initialize audio engine! err code: {}", (int)result);
    }

    Track::Load(track, "Resonance - Home.mp3", &audioEngine);
}

App::~App() {
    ma_engine_uninit(&audioEngine);
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
    uRoot.rect = { 0, (fv2)windowSize };
    uRoot.ComputeLayout();
}

void App::Update() {
    if (gd.GetWindowSize() != windowSize) { // resize happened
        OnScreenResize();
    }
    const auto& io = gd.GetIO();
    uRoot.CheckMouseStates(io.GetMousePos(), io.LeftMouse().Pressed(), io.LeftMouse().OnPress());
}

bool App::Run() {
    Update();

    const float dt = std::min(gd.GetIO().DeltaTime(), 0.333f);
    gd.Begin();
    canvas.BeginFrame();

    const auto& io = gd.GetIO();
    if (io['K'].OnPress() && io.Shift()) {
        return false;
    }

    canvas.NoStroke();
    canvas.Fill(0x272b34_rgb);
    canvas.DrawRect({ 0, { Width(), Height() } });

    uTitlebar.Draw(canvas);
    uSidebar.Draw(canvas);
    uMusic.Draw(canvas, track);
    uPlaybar.Draw(canvas, track, io);

    debugMode ^= io["I"].OnPress() && io.Shift();
    if (debugMode) {
        canvas.StrokeWeight(5);
        canvas.Stroke(1);
        canvas.DrawText(
            Text::Format(
                "WPos = {}, {}x{}; "
                "LClick = {}; "
                "Mouse = {}; "
                "FPS = {}; "
                "MouseDown = {}",
                gd.GetWindowPos(), gd.GetWindowSize().x, gd.GetWindowSize().y,
                io.LeftMouse().ClickedPos(),
                io.GetMousePos(),
                std::llround(io.Framerate()),
                io.LeftMouse().Pressed()
            ),
            11.0f,
            { 0, Height() },
            { .alignment = TextAlign::LEFT | TextAlign::VBOTTOM }
        );

        uRoot.DrawDebug(canvas);
    }

    if (io[IO::Key::SPACE].OnPress()) {
        track.Toggle();
    }

    {
        uSidebar.Update(dt);
    }

    canvas.EndFrame();
    gd.End();

    return gd.WindowIsOpen();
}

String App::FormatSeconds(int seconds) {
    return Text::Format("{}:{:02}", seconds / 60, seconds % 60);
}
