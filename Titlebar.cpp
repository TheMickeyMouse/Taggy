#include "Titlebar.h"

#include "App.h"
#include "Quasi/Dependencies/GLFW/include/GLFW/glfw3.h"

Titlebar::Titlebar(UIRect& root)
    : uRoot(root.Pack(Dir::TOP, HEIGHT * Length::PX)),
      uCloseBtn   (uRoot->Pack(Dir::RIGHT, 1.0_fh)),
      uMaximizeBtn(uRoot->Pack(Dir::RIGHT, 1.0_fh)),
      uMinimizeBtn(uRoot->Pack(Dir::RIGHT, 1.0_fh))
{}

void Titlebar::Draw(Canvas& canvas) {
    const auto _ = uRoot->BeginDraw(canvas);

    canvas.Fill(BACKGROUND);
    canvas.DrawRect({ 0, uRoot->rect.Size() });

    for (auto btn : { uCloseBtn, uMaximizeBtn, uMinimizeBtn }) {
        [[maybe_unused]] const auto _2 = btn->BeginDraw(canvas);
        canvas.transform.scale = btn->rect.Width();

        canvas.Fill(btn->isHovered ? (btn.RefEquals(uCloseBtn) ? RED : DARK_GRAY) : BACKGROUND);
        if (btn->isHovered) {
            canvas.NoStroke();
            canvas.DrawRect({ 0, 1 });
        }

        canvas.Stroke(LIGHT_GRAY);
        canvas.StrokeWeight(0.5f / canvas.transform.scale.x);

        if (btn.RefEquals(uCloseBtn)) { // x icon (close)
            if (btn->isHovered) canvas.Stroke(1);
            canvas.DrawLine(0.4, 0.6);
            canvas.DrawLine({ 0.6, 0.4 }, { 0.4, 0.6 });
        } else if (btn.RefEquals(uMaximizeBtn)) { // double square icon (maximize)
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
        canvas.DrawText("Taggy v1.0.0", 16, uRoot->rect.Center());
    }
}
