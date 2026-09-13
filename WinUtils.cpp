#include "Utils/Debug/Logger.h"
#include "windows.h"
#include "WinUtils.h"
#define GLFW_EXPOSE_NATIVE_WIN32
#include "Quasi/Dependencies/GLFW/include/GLFW/glfw3native.h"
#include "Utils/Math/Vector.h"

namespace WinUtils {
    using namespace Quasi::Math;
    
    WNDPROC originalWinProc;
    int titlebarWidth, titlebarHeight;

    bool InTitlebar(int mouseY) {
        return 0 < mouseY && mouseY < titlebarHeight;
    }
    enum TitlebarBtn {
        NONE, CLOSE = 1, MAXIMIZE, MINIMIZE
    };
    TitlebarBtn WhichBtn(int width, int mouseX) {
        width -= 16; // frame size correction = 2 * GetSystemMetrics(SM_CXFRAME); idk why
        if (mouseX > width - titlebarWidth) {
            return CLOSE;
        }
        if (mouseX > width - 2 * titlebarWidth /* && isMaximizable */) {
            return MAXIMIZE;
        }
        if (mouseX > width - 3 * titlebarWidth /* && isMinimizable */) {
            return MINIMIZE;
        }
        return NONE;
    }
    POINT GetCursor(LPARAM lp) {
        return { LOWORD(lp), HIWORD(lp) };
    } 

    void SetMenuItemState(HMENU menu, MENUITEMINFO* menuiteminfo, UINT item, bool enabled) {
        menuiteminfo->fState = enabled ? MF_ENABLED : MF_DISABLED;
        SetMenuItemInfo(menu, item, false, menuiteminfo);
    }

    // THANK YOU https://github.com/Bastitron/Win32Chrome <3
    // you saved this project
    LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
        switch (uMsg) {
            case WM_NCCALCSIZE: {
                const BOOL isMaximized = IsZoomed(hWnd);
                if (!wParam) {
                    return DefWindowProc(hWnd, uMsg, wParam, lParam);
                }
                //
                int frame_x = GetSystemMetrics(SM_CXFRAME);
                int frame_y = GetSystemMetrics(SM_CYFRAME);
                int padding = GetSystemMetrics(SM_CXPADDEDBORDER);
                // Quasi::Debug::QInfo$("fx = {}, fy = {}, pad = {}", frame_x, frame_y, padding);

                NCCALCSIZE_PARAMS* params = (NCCALCSIZE_PARAMS *)lParam;
                RECT* rcr = params->rgrc;

                rcr->right  -= frame_x + padding;
                rcr->left   += frame_x + padding;
                rcr->bottom -= frame_y + padding;

                if (isMaximized) {
                    rcr->top += frame_y + padding;
                }

                return 0;
            }
            // case WM_CREATE: {
            //     SetWindowPos(hWnd, nullptr, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER);
            //     break;
            // }
            case WM_NCHITTEST: {
                const BOOL isMaximized = IsZoomed(hWnd);
                LRESULT hit = DefWindowProc(hWnd, uMsg, wParam, lParam);
                switch (hit) {
                    case HTNOWHERE:     case HTRIGHT:  case HTLEFT:
                    case HTTOPLEFT:     case HTTOP:    case HTTOPRIGHT:
                    case HTBOTTOMRIGHT: case HTBOTTOM: case HTBOTTOMLEFT: {
                        return hit;
                    }
                    default:;
                }

                POINT cursor = GetCursor(lParam);
                ScreenToClient(hWnd, &cursor);

                RECT rect;
                GetWindowRect(hWnd, &rect);
                int width = rect.right - rect.left;
                if (InTitlebar(cursor.y)) {
                    switch (WhichBtn(width, cursor.x)) {
                        case NONE: break;
                        case CLOSE:    return HTCLOSE;
                        case MAXIMIZE: return HTMAXBUTTON;
                        case MINIMIZE: return HTMINBUTTON;
                    }
                }

                int frame_y = GetSystemMetrics(SM_CYFRAME);
                int padding = GetSystemMetrics(SM_CXPADDEDBORDER);

                if (!isMaximized && cursor.y > 0 && cursor.y < frame_y + padding) {
                    return HTTOP;
                }

                if (InTitlebar(cursor.y) /* && hitTest(cursor) */) {
                    return HTCAPTION;
                }

                return HTCLIENT;
            }
            case WM_NCLBUTTONDOWN: {
                POINT cursor = GetCursor(lParam);
                RECT rect;
                GetWindowRect(hWnd, &rect);
                int width = rect.right - rect.left;
                ScreenToClient(hWnd, &cursor);
                if (InTitlebar(cursor.y) && WhichBtn(width, cursor.x) != NONE) {
                    return 0;
                }
                break;
            }
            //
            case WM_NCLBUTTONUP: {
                POINT cursor = GetCursor(lParam);
                ScreenToClient(hWnd, &cursor);

                RECT rect;
                GetWindowRect(hWnd, &rect);
                int width = rect.right - rect.left;

                if (InTitlebar(cursor.y)) {
                    switch (WhichBtn(width, cursor.x)) {
                        case NONE: break;
                        case CLOSE: {
                            PostMessageW(hWnd, WM_CLOSE, 0, 0);
                            return 0;
                        }
                        case MAXIMIZE: /* if (isMaximizable) */ {
                            const BOOL isMaximized = IsZoomed(hWnd);
                            ShowWindow(hWnd, isMaximized ? SW_NORMAL : SW_MAXIMIZE);
                            return 0;
                        }
                        case MINIMIZE: /* if (isMinimizable) */ {
                            ShowWindow(hWnd, SW_MINIMIZE);
                            return 0;
                        }
                    }
                }
                break;
            }
            // case WM_NCRBUTTONUP: {
            //     if (wParam == HTCAPTION) {
            //         const BOOL isMaximized = IsZoomed(hWnd);
            //         MENUITEMINFO info = {
            //             .cbSize = sizeof(info),
            //             .fMask = MIIM_STATE
            //         };
            //
            //         const HMENU sys_menu = GetSystemMenu(hWnd, false);
            //         SetMenuItemState(sys_menu, &info, SC_RESTORE, isMaximized);
            //         SetMenuItemState(sys_menu, &info, SC_MOVE, !isMaximized);
            //         SetMenuItemState(sys_menu, &info, SC_SIZE, !isMaximized);
            //         SetMenuItemState(sys_menu, &info, SC_MINIMIZE, true);
            //         SetMenuItemState(sys_menu, &info, SC_MAXIMIZE, !isMaximized);
            //         SetMenuItemState(sys_menu, &info, SC_CLOSE, true);
            //
            //         POINT cursor = GetCursor(lParam);
            //         const BOOL result = TrackPopupMenu(sys_menu, TPM_RETURNCMD, cursor.x, cursor.y, 0, hWnd, nullptr);
            //         if (result != 0) {
            //             PostMessage(hWnd, WM_SYSCOMMAND, result, 0);
            //         }
            //     }
            //     break;
            // }
            default:;
        }

        return CallWindowProc(originalWinProc, hWnd, uMsg, wParam, lParam);
    }

    void UseCustomTitlebar(GLFWwindow* window, int titleHeight, int titleWidth) {
        HWND hWnd = glfwGetWin32Window(window);

        LONG_PTR lStyle = GetWindowLongPtr(hWnd, GWL_STYLE);
        lStyle |= WS_THICKFRAME | WS_SYSMENU | WS_VISIBLE | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
        SetWindowLongPtr(hWnd, GWL_STYLE, lStyle);

        RECT windowRect;
        GetWindowRect(hWnd, &windowRect);
        int width = windowRect.right - windowRect.left;
        int height = windowRect.bottom - windowRect.top;

        titlebarHeight = titleHeight;
        titlebarWidth = titleWidth;

        originalWinProc = (WNDPROC)GetWindowLongPtr(hWnd, GWLP_WNDPROC);
        (void)SetWindowLongPtr(hWnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(WindowProc));
        SetWindowPos(hWnd, NULL, 0, 0, width, height, SWP_FRAMECHANGED | SWP_NOMOVE);
    }
}
