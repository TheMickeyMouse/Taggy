#pragma once
#include "Track.h"
#include "Quasi/Dependencies/GLFW/include/GLFW/glfw3.h"

using namespace Quasi;

namespace WinUtils {
    void SetHwnd(GLFWwindow* window);

    void UseCustomTitlebar(int titleHeight, int titleWidth);
    Vec<Path> OpenFileDialog();
}
