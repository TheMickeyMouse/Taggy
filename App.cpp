#include "App.h"

#include <thread>

#include "glp.h"
#include "WinUtils.h"

App::App(int w, int h)
    : gd(GraphicsDevice::Initialize({ w, h }, { .beginPosition = { 300 }, .windowTitle = "Taggy" })),
      canvas(gd), windowSize { w, h },

      font(Font::LoadFile("C:/Users/User/AppData/Local/Microsoft/Windows/Fonts/JetBrainsMono-Regular.ttf", 48.0f)),
      fontBold(Font::LoadFile("C:/Users/User/AppData/Local/Microsoft/Windows/Fonts/JetBrainsMono-Bold.ttf", 48.0f)),

      uDoc({ 0, (fv2)windowSize }),
      uTitlebar(uDoc.Root()),
      uPlaybar (uDoc.Root(), canvas),
      uSidebar (uDoc.Root(), canvas),
      uMusic   (uDoc.Root())
{
    glfwSetWindowSizeLimits(gd.GetWindow(), 800, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
    WinUtils::UseCustomTitlebar(gd.GetWindow(), Titlebar::HEIGHT, Titlebar::HEIGHT);
    canvas.FlipYDirection();

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
    uDoc.SetRect({ 0, (fv2)windowSize });
    uDoc.ComputeLayout();
}

void App::Update() {
    if (gd.GetWindowSize() != windowSize) { // resize happened
        OnScreenResize();
    }
}

bool App::Run() {
    Update();

    const float dt = std::min(gd.GetIO().DeltaTime(), 0.333f);
    gd.Begin();

    if (glfwGetWindowAttrib(gd.GetWindow(), GLFW_FOCUSED)) {
        gd.PollEvents();
    }

    if (!IsWindowMinimized()) {
        canvas.BeginFrame();

        const auto& io = gd.GetIO();

        uDoc.CheckMouseStates(io.GetMousePos(), io.LeftMouse().Pressed(), io.LeftMouse().OnPress());

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

            uDoc.DrawDebug(canvas);
        }

        if (io[IO::Key::SPACE].OnPress()) {
            track.Toggle();
        }

        {
            uSidebar.Update(dt);
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
