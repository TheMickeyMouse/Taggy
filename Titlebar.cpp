#include "Titlebar.h"

#include "App.h"
#include "Quasi/Dependencies/GLFW/include/GLFW/glfw3.h"

Titlebar::Titlebar(UIRect& root)
    : uRoot       (root  .Pack(Dir::TOP, HEIGHT * Length::PX, "ttb")),
      uCloseBtn   (uRoot->Pack(Dir::RIGHT, 1.0_fh, "ttb.CloseBtn")),
      uMaximizeBtn(uRoot->Pack(Dir::RIGHT, 1.0_fh, "ttb.MaximizeBtn")),
      uMinimizeBtn(uRoot->Pack(Dir::RIGHT, 1.0_fh, "ttb.MinimizeBtn"))
{}

void Titlebar::Draw(Canvas& canvas) {
    const auto _ = uRoot->BeginDraw(canvas);

    for (auto btn : { uCloseBtn, uMaximizeBtn, uMinimizeBtn }) {
        [[maybe_unused]] const auto _2 = btn->BeginDraw(canvas);
        canvas.transform.scale = btn->rect.Width();

        const bool hov = btn->Hovered();
        canvas.Fill(hov ? (btn == uCloseBtn ? RED : DARK_GRAY) : BACKGROUND);
        if (hov) {
            canvas.NoStroke();
            canvas.DrawRect({ 0, 1 });
        }

        canvas.Stroke(LIGHT_GRAY);
        canvas.StrokeWeight(0.5f / canvas.transform.scale.x);

        if (btn == uCloseBtn) { // x icon (close)
            if (hov) canvas.Stroke(1);
            canvas.DrawLine(0.4, 0.6);
            canvas.DrawLine({ 0.6, 0.4 }, { 0.4, 0.6 });
        } else if (btn == uMaximizeBtn) { // double square icon (maximize)
            canvas.StrokeJoin(UIRender::MITER_JOIN);
            if (glfwGetWindowAttrib(GraphicsDevice::GetMainWindow(), GLFW_MAXIMIZED)) {
                canvas.DrawRect({ { 0.43, 0.4 }, { 0.6, 0.57 } });
                canvas.DrawRect({ { 0.4, 0.45 }, { 0.55, 0.6 } });
            } else {
                canvas.DrawRect({ 0.4, 0.6 });
            }
            canvas.StrokeJoin(UIRender::ROUND_JOIN);
        } else { // line icon (minimize)
            canvas.DrawLine({ 0.4, 0.5 }, { 0.6, 0.5 });
        }
    }

    {
        canvas.Stroke(LIGHT_GRAY);
        canvas.SetFont(App::GetFontBold());
        canvas.DrawText("Taggy v1.0.0", 16, uRoot->rect.Center());
        canvas.SetFont(App::GetFont());
    }
}
