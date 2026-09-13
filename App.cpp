#include "App.h"

#include <thread>

#include "WinUtils.h"

App::App(int w, int h, bool debug)
    : gd(GraphicsDevice::Initialize({ w, h }, { .beginPosition = { 300 }, .windowTitle = "Taggy" })),
      canvas(gd), windowSize { w, h },
      debugMode(debug) {
    glfwSetWindowSizeLimits(gd.GetWindow(), 800, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
    WinUtils::UseCustomTitlebar(gd.GetWindow(), TITLEBAR_HEIGHT, TITLEBAR_HEIGHT);
    canvas.FlipYDirection();
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

void App::DrawWindowBtns() {
    [[maybe_unused]] const auto _ = canvas.PushTransform();
    canvas.transform.scale = TITLEBAR_HEIGHT;
    DrawCloseBtn(0);
    DrawCloseBtn(1);
    DrawCloseBtn(2);
}

void App::DrawCloseBtn(int btnNum) {
    const float x = Width() - TITLEBAR_HEIGHT * (btnNum + 1);
    canvas.transform.pos = { x, 0 };
    const fRect2D rect = { { x + 1, 0 }, { x + TITLEBAR_HEIGHT, TITLEBAR_HEIGHT } };

    enum BtnType { CLOSE, FULLSCREEN, MINIMIZE } type = (BtnType)btnNum;

    const auto& io = gd.GetIO();
    const bool hover = io.GetMousePos().IsIn(rect);

    canvas.Fill(hover ? type == CLOSE ? 0xc51f1f_rgb : 0x5c6370_rgb : 0x171a1f_rgb);
    if (hover) {
        canvas.NoStroke();
        canvas.DrawRect({ 0, 1 });
    }

    canvas.Stroke(0xabb2bf_rgb);
    canvas.StrokeWeight(0.5f / TITLEBAR_HEIGHT);

    switch (type) {
        case CLOSE: // x
            canvas.DrawLine(0.4, 0.6);
            canvas.DrawLine({ 0.6, 0.4 }, { 0.4, 0.6 });
            break;
        case FULLSCREEN:
            canvas.StrokeJoin(UIRender::MITER_JOIN);
            if (glfwGetWindowAttrib(Window(), GLFW_MAXIMIZED)) {
                canvas.DrawRect({ { 0.43, 0.4 }, { 0.6, 0.57 } });
                canvas.DrawRect({ { 0.4, 0.45 }, { 0.55, 0.6 } });
            } else {
                canvas.DrawRect({ 0.4, 0.6 });
            }
            canvas.StrokeJoin(UIRender::ROUND_JOIN);
            break;
        case MINIMIZE:
            canvas.DrawLine({ 0.4, 0.5 }, { 0.6, 0.5 });
            break;
    }
}

void App::Update() {
    if (gd.GetWindowSize() != windowSize) { // resize happened
        windowSize = gd.GetWindowSize();
        canvas.SetViewport({ { 0, (float)windowSize.y }, { (float)windowSize.x, 0 } });
    }
}

bool App::Run() {
    Update();
    gd.Begin();
    gd.ClearColor(0x272b34_rgb);
    canvas.BeginFrame();

    const auto& io = gd.GetIO();
    if (io['K'].OnPress()) {
        return false;
    }

    canvas.NoStroke();
    canvas.Fill(0x171a1f_rgb);
    canvas.DrawRect({ 0, { Width(), TITLEBAR_HEIGHT } });
    DrawWindowBtns();

    canvas.StrokeWeight(5);
    canvas.Stroke(1);
    canvas.DrawPoint(io.GetMousePos());
    canvas.DrawText(Text::Format("WPos = {}", gd.GetWindowPos()), 20.0f, { 0, Height() - 66 }, { .alignment = TextAlign::LEFT });
    canvas.DrawText(Text::Format("LClick = {}", io.LeftMouse().ClickedPos()), 20.0f, { 0, Height() - 48 }, { .alignment = TextAlign::LEFT });
    canvas.DrawText(Text::Format("Mouse = {}", io.GetMousePos()), 20.0f, { 0, Height() - 30 }, { .alignment = TextAlign::LEFT });
    canvas.DrawText(Text::Format("FPS = {}", io.Framerate()), 20.0f, { 0, Height() - 12 }, { .alignment = TextAlign::LEFT });

    canvas.EndFrame();
    gd.End();

    return gd.WindowIsOpen();
}
