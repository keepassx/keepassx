#include "AutoTypeWindows.h"

#define MAX_WINDOW_TITLE_LENGTH 1024

AutoTypePlatformWin::AutoTypePlatformWin()
    : m_threadId(0)
{
}

//
// Windows 7 or later
// see: https://msdn.microsoft.com/en-us/library/windows/desktop/ms724451%28v=vs.85%29.aspx
//
bool AutoTypePlatformWin::isAvailable()
{
    OSVERSIONINFO osvi;

    ::ZeroMemory(&osvi, sizeof(OSVERSIONINFO));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);

    ::GetVersionEx(&osvi);

    return  ( (osvi.dwMajorVersion > 6) ||
            ( (osvi.dwMajorVersion == 6) && (osvi.dwMinorVersion >= 1) ));
}

//
// Get list of all visible window titles
//
QStringList AutoTypePlatformWin::windowTitles()
{
    QStringList list;

    ::EnumWindows(AutoTypePlatformWin::windowTitleEnumProc, reinterpret_cast<LPARAM>(&list));

    return list;
}

//
// Get foreground window hwnd
//
WId AutoTypePlatformWin::activeWindow()
{
    return reinterpret_cast<WId>(::GetForegroundWindow());
}

//
// Get foreground window title
//
QString AutoTypePlatformWin::activeWindowTitle()
{
    wchar_t title[MAX_WINDOW_TITLE_LENGTH];
    int count = ::GetWindowTextW(::GetForegroundWindow(), title, MAX_WINDOW_TITLE_LENGTH);

    return QString::fromUtf16(reinterpret_cast<const ushort *>(title), count);
}

//
// Start hotkey thread
//
bool AutoTypePlatformWin::registerGlobalShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    if (m_threadId == 0) {
        m_key = key;
        m_modifiers = modifiers;

        HANDLE thread = ::CreateThread(nullptr, 0, AutoTypePlatformWin::hotkeyThreadProc, this, 0, &m_threadId);
        ::CloseHandle(thread);

        return true;
    }

    return false;
}

//
// Stop hotkey thread
//
void AutoTypePlatformWin::unregisterGlobalShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(key);
    Q_UNUSED(modifiers);

    if (m_threadId != 0) {
        // Signal graceful shutdown
        ::PostThreadMessage(m_threadId, WM_QUIT, 0, 0);
    }
}

//
// Native event filter
//
int AutoTypePlatformWin::platformEventFilter(void* event)
{
    Q_UNUSED(event);

    // Called from nativeEventFilter in core/Application.cpp
    // if eventType == "xcb_generic_event_t"
    Q_ASSERT(false);

    return -1;
}

AutoTypeExecutor* AutoTypePlatformWin::createExecutor()
{
    return new AutoTypeExecutorWin(this);
}

int AutoTypePlatformWin::initialTimeout()
{
    return 500;
}

//
// Set foreground window
//
bool AutoTypePlatformWin::raiseWindow(WId window)
{
    HWND hwnd = reinterpret_cast<HWND>(window);
    return ::BringWindowToTop(hwnd) && ::SetForegroundWindow(hwnd);
}

//
// Send unicode character to foreground window
//
void AutoTypePlatformWin::sendChar(const QChar& ch)
{
    INPUT in;
    in.type = INPUT_KEYBOARD;
    in.ki.wVk = 0;
    in.ki.wScan = ch.unicode();
    in.ki.dwFlags = KEYEVENTF_UNICODE;
    in.ki.time = 0;
    in.ki.dwExtraInfo = ::GetMessageExtraInfo();

    ::SendInput(1, &in, sizeof(INPUT));
    ::Sleep(25);
}

//
// Send virtual key code to foreground window
//
void AutoTypePlatformWin::sendKey(Qt::Key key)
{
    BYTE wVk = getNativeKeyCode(key);
    if (wVk == BYTE(-1)) {
        return; // No symbol
    }

    INPUT in;
    in.type = INPUT_KEYBOARD;
    in.ki.wVk = wVk;
    in.ki.wScan = ::MapVirtualKeyW(wVk, MAPVK_VK_TO_VSC);
    in.ki.dwFlags = 0;
    in.ki.time = 0;
    in.ki.dwExtraInfo = ::GetMessageExtraInfo();

    ::SendInput(1, &in, sizeof(INPUT));
    ::Sleep(25);
}

//
// Translate qt key code to windows virtual key code
//
BYTE AutoTypePlatformWin::getNativeKeyCode(Qt::Key key)
{
    switch (key) {
    case Qt::Key_Tab:
        return VK_TAB;
    case Qt::Key_Enter:
        return VK_RETURN;
    case Qt::Key_Up:
        return VK_UP;
    case Qt::Key_Down:
        return VK_DOWN;
    case Qt::Key_Left:
        return VK_LEFT;
    case Qt::Key_Right:
        return VK_RIGHT;
    case Qt::Key_Insert:
        return VK_INSERT;
    case Qt::Key_Delete:
        return VK_DELETE;
    case Qt::Key_Home:
        return VK_HOME;
    case Qt::Key_End:
        return VK_END;
    case Qt::Key_PageUp:
        return VK_PRIOR;
    case Qt::Key_PageDown:
        return VK_NEXT;
    case Qt::Key_Backspace:
        return VK_BACK;
    case Qt::Key_Pause:
        return VK_PAUSE;
    case Qt::Key_CapsLock:
        return VK_CAPITAL;
    case Qt::Key_Escape:
        return VK_ESCAPE;
    case Qt::Key_Help:
        return VK_HELP;
    case Qt::Key_NumLock:
        return VK_NUMLOCK;
    case Qt::Key_Print:
        return VK_PRINT;
    case Qt::Key_ScrollLock:
        return VK_SCROLL;
    default:
        if (key >= Qt::Key_F1 && key <= Qt::Key_F16) {
            return VK_F1 + (key - Qt::Key_F1);
        }
        else {
            Q_ASSERT(false);
            return BYTE(-1);
        }
    }
}

//
// see: https://stackoverflow.com/questions/7277366/why-does-enumwindows-return-more-windows-than-i-expected
//      https://blogs.msdn.microsoft.com/oldnewthing/20071008-00/?p=24863
//
BOOL AutoTypePlatformWin::isAltTabWindow(HWND hwnd)
{
    if (!::IsWindowVisible(hwnd)) {
        return FALSE;
    }

    // Start at the root owner
    HWND hwndWalk = ::GetAncestor(hwnd, GA_ROOTOWNER);
    HWND hwndTry = nullptr;

    while (hwndTry != hwndWalk) {
        // See if we are the last active visible popup
        hwndTry = ::GetLastActivePopup(hwndWalk);
        if(::IsWindowVisible(hwndTry)) {
            break;
        }
        hwndWalk = hwndTry;
    }

    return hwndWalk == hwnd;
}

//
// WinApi enum proc
//
BOOL CALLBACK AutoTypePlatformWin::windowTitleEnumProc(
    _In_ HWND   hwnd,
    _In_ LPARAM lParam
)
{
    if (!isAltTabWindow(hwnd)) {
        // Skip window
        return TRUE;
    }

    wchar_t title[MAX_WINDOW_TITLE_LENGTH];
    QStringList * list = reinterpret_cast<QStringList *>(lParam);
    int count = ::GetWindowTextW(hwnd, title, MAX_WINDOW_TITLE_LENGTH);

    if (list != nullptr && count > 0) {
        // Add window title
        list->append(QString::fromUtf16(reinterpret_cast<const ushort *>(title), count));
    }

    return TRUE;
}

//
// Hotkey thread proc
//
DWORD WINAPI AutoTypePlatformWin::hotkeyThreadProc(
  _In_ LPVOID lpParameter
)
{
    AutoTypePlatformWin *self = reinterpret_cast<AutoTypePlatformWin *>(lpParameter);

    // Register global hotkey
    #define MOD_NOREPEAT 0x4000 // Missing in MinGW
    if (!::RegisterHotKey(nullptr, 1, MOD_CONTROL | MOD_NOREPEAT, /* Ctrl+T: */ 0x54)) {
        return 1;
    }

    MSG msg;
    ::ZeroMemory(&msg, sizeof(MSG));

    while (::GetMessage(&msg, nullptr, 0, 0) != 0)
    {
        if (msg.message == WM_QUIT) {
            // Exit thread
            break;
        }
        else if (msg.message == WM_HOTKEY) {
            // Emit hotkey signal
            Q_EMIT self->globalShortcutTriggered();
        }
    }

    ::UnregisterHotKey(nullptr, 1);

    return 0;
}

//
// ------------------------------ AutoTypeExecutorWin ------------------------------
//

AutoTypeExecutorWin::AutoTypeExecutorWin(AutoTypePlatformWin* platform)
    : m_platform(platform)
{
}

void AutoTypeExecutorWin::execChar(AutoTypeChar* action)
{
    m_platform->sendChar(action->character);
}

void AutoTypeExecutorWin::execKey(AutoTypeKey* action)
{
    m_platform->sendKey(action->key);
}

