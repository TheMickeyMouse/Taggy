#include "Titlebar.h"

#include "App.h"
#include "Quasi/Dependencies/GLFW/include/GLFW/glfw3.h"

Titlebar::Titlebar(UIRect& titlebar)
    : bar(titlebar),
      closeBtn   (titlebar.Pack(Dir::RIGHT, 1.0_fh)),
      maximizeBtn(titlebar.Pack(Dir::RIGHT, 1.0_fh)),
      minimizeBtn(titlebar.Pack(Dir::RIGHT, 1.0_fh))
{}

void Titlebar::Draw(Canvas& canvas) {
    const auto _ = bar->BeginDraw(canvas);

    canvas.Fill(0x171a1f_rgb);
    canvas.DrawRect({ 0, bar->rect.Size() });

    static const auto
        RED   = 0xc51f1f_rgb,
        LGRAY = 0x5c6370_rgb,
        DGRAY = 0x171a1f_rgb,
        WHITE = 0xabb2bf_rgb;

    for (auto btn : { closeBtn, maximizeBtn, minimizeBtn }) {
        [[maybe_unused]] const auto _2 = btn->BeginDraw(canvas);
        canvas.transform.scale = btn->rect.Width();

        canvas.Fill(btn->isHovered ? (btn.RefEquals(closeBtn) ? RED : LGRAY) : DGRAY);
        if (btn->isHovered) {
            canvas.NoStroke();
            canvas.DrawRect({ 0, 1 });
        }

        canvas.Stroke(WHITE);
        canvas.StrokeWeight(0.5f / canvas.transform.scale.x);

        if (btn.RefEquals(closeBtn)) { // x icon (close)
            if (btn->isHovered) canvas.Stroke(1);
            canvas.DrawLine(0.4, 0.6);
            canvas.DrawLine({ 0.6, 0.4 }, { 0.4, 0.6 });
        } else if (btn.RefEquals(maximizeBtn)) { // double square icon (maximize)
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
        canvas.Stroke(WHITE);
        // canvas.Stroke(0xdcddde_rgb);
        // [[maybe_unused]] const auto _ = canvas.BeginShadow({ 0, 4 }, 0.0f, 0);
        canvas.DrawText("Taggy v1.0.0", 16, bar->rect.Center());
    }
}
