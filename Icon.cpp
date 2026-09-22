#include "Icon.h"

Icon Icon::FromSVG(Str src, Canvas& canvas) {
    Icon icon;
    [[maybe_unused]] const auto _ = canvas.RenderTo(icon.mesh);
    [[maybe_unused]] const auto _2 = canvas.PushStyles();

    canvas.NoFill();
    canvas.Stroke(1);

    ParseSVGHeader(src, icon, canvas);

    while (src) {
        src = src.TrimStart();
        if (src.StartsWith("<path d=\"")) {
            src = src.RemovePrefix("<path d=\"");
            const auto [path, rest] = src.SplitOnce('"');
            [[maybe_unused]] Str dummy;
            rest.SplitOnce("/>").TieTo(dummy, src);

            ParsePath(path, canvas);
        } else if (src.StartsWith("<circle")) {
            src = src.RemovePrefix("<circle");

            fv2 center;
            float radius = 1;
            while (!src.StartsWith("/>")) {
                src = src.TrimStart();

                auto [propName, value] = src.SplitOnce('=');
                Debug::QAssert$(value[0] == '"', "no opening quotation mark");
                value.Advance(1);
                value.SplitOnce('"').TieTo(value, src);

                if (propName == "cx") {
                    center.x = Text::Parse<float>(value).Assert();
                } else if (propName == "cy") {
                    center.y = Text::Parse<float>(value).Assert();
                } else if (propName == "r") {
                    radius = Text::Parse<float>(value).Assert();
                } else {
                    Debug::QWarn$("skipping property name {} for circle", propName);
                }
            }
            src.Advance(2); // "/>"
            canvas.DrawCircle(center, radius);
        } else {
            Debug::QError$("bad object");
            return icon;
        }
    }

    return icon;
}

void Icon::ParseSVGHeader(Str& src, Icon& icon, Canvas& canvas) {
    src = src.Trim();
    Debug::QAssert$(src.StartsWith("<svg"), "bad svg header! src: {}", src);
    src.Advance(4);
    while (true) {
        src = src.TrimStart();
        if (src[0] == '>') break;

        auto [propName, value] = src.SplitOnce('=');
        Debug::QAssert$(value[0] == '"', "no opening quotation mark");
        value.Advance(1);
        value.SplitOnce('"').TieTo(value, src);

        if (propName == "xmlns") Debug::QAssert$(value == "http://www.w3.org/2000/svg", "bad xmlns");
        else if (propName == "width" || propName == "height" ||
                 propName == "fill" || propName == "stroke" || propName == "class") continue; // we dont need this lol
        else if (propName == "viewBox") {
            iv2 min, size;
            // temporary solution; easy and simple :)
            std::sscanf(value.Data(), "%d %d %d %d", &min.x, &min.y, &size.x, &size.y);
            icon.viewBox = fRect2D::FromSize((fv2)min, (fv2)size);
        } else if (propName == "stroke-width") {
            canvas.StrokeWeight(Text::Parse<float>(value).Assert("bad stroke weight value") / 2.0f);
        } else if (propName == "stroke-linecap") {
            if      (value == "round")  canvas.StrokeCap(UIRender::ROUND_CAP);
            else if (value == "square") canvas.StrokeCap(UIRender::SQUARE_CAP);
            else if (value == "butt")   canvas.StrokeCap(UIRender::FLAT_CAP);
            else Debug::QError$("bad stroke cap value '{}'", value);
        } else if (propName == "stroke-linejoin") {
            if      (value == "round") canvas.StrokeCap(UIRender::ROUND_JOIN);
            else if (value == "bevel") canvas.StrokeCap(UIRender::BEVEL_JOIN);
            else if (value == "miter") canvas.StrokeCap(UIRender::MITER_JOIN);
            else Debug::QError$("bad stroke join value '{}'", value);
        } else {
            Debug::QWarn$("skipping property {}", propName);
        }
    }
    src = src.RemovePrefix('>').RemoveSuffix("</svg>");
}

void Icon::ParsePath(Str src, Canvas& canvas) {
    // https://developer.mozilla.org/en-US/docs/Web/SVG/Tutorials/SVG_from_scratch/Paths
    // COMMANDS:
    // M x y: new path starting at (x, y)
    // L x y: connect to (x, y)
    // H x:   same as L but y = 0
    // V y:   same as L but x = 0
    // Z:     close path
    // A rx ry xrot large-arc? sweep? x y: arc
    // UNSUPPORTED: C Q S T
    Option<Canvas::Path> path = nullptr;
    fv2 currPos;
    bool isRelative;

    const auto GetPos = [&] (Option<float> x, Option<float> y) {
        if (isRelative) {
            if (x) currPos.x += *x;
            if (y) currPos.y += *y;
        } else {
            if (x) currPos.x = *x;
            if (y) currPos.y = *y;
        }
        return currPos;
    };

    const auto ParseNum = [&] (const char* propertyName) -> float {
        const auto [n, x] = Text::ParsePartial<float>(src);
        if (!n) {
            Debug::QError$("bad value for {}", propertyName);
            return 0;
        }

        src = src.Skip(*n).TrimStart();
        return *x;
    };

    const auto ParseFlag = [&] (const char* propertyName) -> bool {
        switch (src[0]) {
            case '0': src = src.Skip(1).TrimStart(); return false;
            case '1': src = src.Skip(1).TrimStart(); return true;
            default: {
                Debug::QError$("bad value for {}", propertyName);
                return false;
            }
        }
    };

    char command;
    while (src) {
        const char newCommand = src[0];

        if (Chr::IsDigit(newCommand) || newCommand == '-') {
            if (command == 'A' || command == 'a') {
                goto arcCommand;
            } else goto lineCommand;
        }

        src = src.Skip(1).TrimStart();
        command = newCommand;
        isRelative = Chr::IsLower(command);

        switch (Chr::ToUpper(command)) {
            case 'M': {
                const float x = ParseNum("x"), y = ParseNum("y");

                if (path) path->DontClosePath();
                path = canvas.NewPath(Canvas::CLOSED_CURVE);
                path->AddPoint(GetPos(x, y));

                break;
            }
            case 'L': {
                lineCommand:
                const float x = ParseNum("x"), y = ParseNum("y");
                path->AddPoint(GetPos(x, y));
                break;
            }
            case 'H': {
                const float x = ParseNum("x");
                path->AddPoint(GetPos(x, nullptr));
                break;
            }
            case 'V': {
                const float y = ParseNum("y");
                path->AddPoint(GetPos(nullptr, y));
                break;
            }
            case 'Z': {
                path = nullptr; // automatically closes
                break;
            }
            case 'A': {
                arcCommand:
                const float rx = ParseNum("rx");
                Debug::QAssert$(ParseNum("ry") == rx, "ellipses (unequal radii) are not supported");
                Debug::QAssert$(ParseNum("xrot") == 0, "arc rotations are not supported.");

                const bool largeArc = ParseFlag("large-arc"), cw = ParseFlag("cw");

                const float x = ParseNum("x"), y = ParseNum("y");

                // let P be the starting point and Q be the end point, O being the center of the arc.
                // OQ = OP = r; => O lies on perp. bisector of PQ.
                // there are actually 2 solutions for O; O₁ and O₂, chosen from the `cw` flag.
                // let PO = PQ * z. z must be of the form 0.5 + ui (remember perp. bisector?)
                // PO = PQ * (0.5 ± ui), |PO| = r; => |0.5 + ui| = |O|/|PQ|, u = √(r²/|PQ|² - 0.25)
                const fv2 PQ = isRelative ? fv2 { x, y } : (fv2 { x, y } - currPos);
                const float u = std::sqrt(rx * rx / PQ.LenSq() - 0.25f);
                const fv2 z = { 0.5f, (cw ^ largeArc) ? u : -u };

                const fv2 PO = PQ.ComplexMul(z);
                const fv2 angle = z.ComplexMul(z);

                // Debug::QInfo$("arc: {} {}", largeArc, cw);
                path->AddCircularArc(currPos + PO,
                    Rotor2D::FromUnitVector(angle.Conj() / -z.LenSq()),
                    largeArc ? Canvas::MAJOR : Canvas::MINOR);
                currPos += PQ;
                break;
            }
            case 'C': case 'Q': case 'T': case 'S': {
                Debug::QWarn$("bezier curves are not yet supported. skipping");
                break;
            }
            default: {
                Debug::QError$("bad command '{}'", command);
            }
        }
    }

    if (path) path->DontClosePath();
}
