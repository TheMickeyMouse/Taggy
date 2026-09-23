#include "Sidebar.h"

#include "glp.h"
#include "Utils/Iter/Map.h"

Sidebar::Sidebar(UIRect& side, Canvas& canvas) : side(side) {
    side.padding = 10;

    labels.Push({ "Home",     Icon::FromSVG(R"(<svg viewBox="0 0 24 24" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M15 21v-8a1 1 0 0 0-1-1h-4a1 1 0 0 0-1 1v8"/><path d="M3 10a2 2 0 0 1 .709-1.528l7-6a2 2 0 0 1 2.582 0l7 6A2 2 0 0 1 21 10v9a2 2 0 0 1-2 2H5a2 2 0 0 1-2-2z"/></svg>)", canvas) });
    labels.Push({ "Library",  Icon::FromSVG(R"(<svg viewBox="0 0 24 24" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="m16 6 4 14"/><path d="M12 6v14"/><path d="M8 8v12"/><path d="M4 4v16"/></svg>)", canvas) });
    labels.Push({ "Search",   Icon::FromSVG(R"(<svg viewBox="0 0 24 24" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="m21 21-4.34-4.34"/><circle cx="11" cy="11" r="8"/></svg>)", canvas) });
    labels.Push({ "Tags",     Icon::FromSVG(R"(<svg viewBox="0 0 24 24" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M13.172 2a2 2 0 0 1 1.414.586l6.71 6.71a2.4 2.4 0 0 1 0 3.408l-4.592 4.592a2.4 2.4 0 0 1-3.408 0l-6.71-6.71A2 2 0 0 1 6 9.172V3a1 1 0 0 1 1-1z"/><path d="M2 7v6.172a2 2 0 0 0 .586 1.414l6.71 6.71a2.4 2.4 0 0 0 3.191.193"/><circle cx="10.5" cy="6.5" r=".5"/></svg>)", canvas) });
    labels.Push({ "History",  Icon::FromSVG(R"(<svg viewBox="0 0 24 24" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M3 12a9 9 0 1 0 9-9 9.75 9.75 0 0 0-6.74 2.74L3 8"/><path d="M3 3v5h5"/><path d="M12 7v5l4 2"/></svg>)", canvas) });
    labels.Push({ "Settings", Icon::FromSVG(R"(<svg viewBox="0 0 24 24" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M9.671 4.136a2.34 2.34 0 0 1 4.659 0 2.34 2.34 0 0 0 3.319 1.915 2.34 2.34 0 0 1 2.33 4.033 2.34 2.34 0 0 0 0 3.831 2.34 2.34 0 0 1-2.33 4.033 2.34 2.34 0 0 0-3.319 1.915 2.34 2.34 0 0 1-4.659 0 2.34 2.34 0 0 0-3.32-1.915 2.34 2.34 0 0 1-2.33-4.033 2.34 2.34 0 0 0 0-3.831A2.34 2.34 0 0 1 6.35 6.051a2.34 2.34 0 0 0 3.319-1.915"/><circle cx="12" cy="12" r="3"/></svg>)", canvas) });

    for (auto& label : labels) {
        side.Pack(Dir::TOP, 1.0_fw);
        label.vActive.smoothness = 12.0f;
    }

    Debug::QInfo$("{} triangles in {} icons", labels.Iter().Map([] (const auto& m) { return m.icon.mesh.indices.Length(); }).Reduce(Operators::Add {}, 0), labels.Length());

    vSelectedIndex.smoothness = 10.0f;
    vHasSelected.smoothness = 6.0f;
}

void Sidebar::Update(float dt) {
    vSelectedIndex.Update(dt);
    vHasSelected.Update(dt);
    for (auto& [_, _2, vActive] : labels) {
        vActive.Update(dt);
    }
}

void Sidebar::Draw(Canvas& canvas) {
    // side->length.val = std::lerp(side->length.val, side->isHovered ? 80.0f : 60.0f, 0.05f);

    const auto _ = side->BeginDraw(canvas);
    canvas.NoStroke();
    canvas.Fill(0x20242b_rgb);
    canvas.DrawRect({ 0, side->rect.Size() });

    {
        const float t = vHasSelected.displayValue, w = side->rect.Width();
        canvas.Fill(0x5c6370_rgbf .AddAlpha(t * 0.5f));
        canvas.DrawCircle({ 0.5f * w, (vSelectedIndex.displayValue + 0.5f) * (w - 2 * side->padding.x) + side->padding.y }, 0.4f * t * w);
    }

    canvas.NoFill();
    canvas.Stroke(0xabb2bf_rgb);

    for (int i = 0; i < labels.Length(); ++i) {
        const auto& u = side->children[i];
        auto& [name, icon, vActive] = labels[i];

        [[maybe_unused]] const auto _2 = u->BeginDraw(canvas);

        const float y = u->rect.Height() / 2.0f, h = y * (2.0f + 0.4f * vActive.displayValue);

        vActive.target = u->isHovered;
        if (u->isHovered) {
            vSelectedIndex.target = i;
            if (!vHasSelected.target) vSelectedIndex.Jump();
        }

        const float t = vActive.displayValue;
        const fColor highlightColor = 0xabb2bf_rgbf .Lerp(0xdcddde_rgbf, t);
        {
            [[maybe_unused]] const auto _3 = canvas.PushTransform();

            const fv2 s = h / icon.viewBox.Size();
            canvas.transform.scale = s * 0.6f;
            canvas.transform.pos += y - 0.3f * h;
            canvas.DrawMesh(icon.mesh, highlightColor);
        }

        if (!vActive.target && vActive.IsStable()) continue;

        canvas.Stroke(highlightColor);
        canvas.DrawText(name, h * 0.5f, { h, 0 }, { TextAlign::LEFT | TextAlign::CLIP, { t * 150.0f, y * 2.0f } });
    }
    vHasSelected.target = side->isChildrenHovered;
}
