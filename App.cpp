#include "App.h"

#include <thread>

#include "glp.h"
#include "WinUtils.h"

App::App(int w, int h)
    : gd(GraphicsDevice::Initialize({ w, h }, { .beginPosition = { 300 }, .windowTitle = "Taggy" })),
      canvas(gd), windowSize { w, h },

      root(UIRect::Root({ 0, (fv2)windowSize })),
      titlebar(root.Pack(Dir::TOP, TITLEBAR_HEIGHT * Length::PX)),
      sidebar(root.Pack(Dir::LEFT, (48.0f + 20.0f) * Length::PX), canvas)
{

    glfwSetWindowSizeLimits(gd.GetWindow(), 800, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
    WinUtils::UseCustomTitlebar(gd.GetWindow(), TITLEBAR_HEIGHT, TITLEBAR_HEIGHT);
    canvas.FlipYDirection();
    canvas.SetFont(font = Font::LoadFile("C:/Users/User/AppData/Local/Microsoft/Windows/Fonts/JetBrainsMono-Bold.ttf", 48.0f));
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
    root.rect = { 0, (fv2)windowSize };
    root.ComputeLayout();
}

void App::Update() {
    if (gd.GetWindowSize() != windowSize) { // resize happened
        OnScreenResize();
    }
    root.CheckHover(gd.GetIO().GetMousePos());
}

bool App::Run() {
    Update();

    gd.Begin();
    // gd.ClearColor(0x272b34_rgb);
    canvas.BeginFrame();

    const auto& io = gd.GetIO();
    if (io['K'].OnPress()) {
        return false;
    }

    canvas.NoStroke();
    canvas.Fill(0x272b34_rgb);
    canvas.DrawRect({ 0, { Width(), Height() } });

    titlebar.Draw(canvas);
    sidebar.Draw(canvas);

    debugMode ^= io["I"].OnPress() && io.Shift();
    if (debugMode) {
        canvas.StrokeWeight(5);
        canvas.Stroke(1);
        // canvas.DrawPoint(io.GetMousePos());
        canvas.DrawText(
            Text::Format(
                "WPos   = {}; {}x{}\n"
                "LClick = {}\n"
                "Mouse  = {}\n"
                "FPS    = {}\n",
                gd.GetWindowPos(), gd.GetWindowSize().x, gd.GetWindowSize().y,
                io.LeftMouse().ClickedPos(),
                io.GetMousePos(),
                std::llround(io.Framerate())
            ),
            20.0f,
            { 0, Height() - 40 },
            { .alignment = TextAlign::LEFT }
        );
    }

    canvas.EndFrame();
    gd.End();

    return gd.WindowIsOpen();
}
