#include "AutoTypeWindows.h"

#define HOTKEY_ID 1
#define MAX_WINDOW_TITLE_LENGTH 1024

#define MOD_NOREPEAT 0x4000 // Missing in MinGW

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
// Register global hotkey
//
bool AutoTypePlatformWin::registerGlobalShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    DWORD nativeKeyCode = qtToNativeKeyCode(key);
    if (nativeKeyCode == 0) {
        return false;
    }
    DWORD nativeModifiers = qtToNativeModifiers(modifiers);
    if (!::RegisterHotKey(nullptr, HOTKEY_ID, nativeModifiers | MOD_NOREPEAT, nativeKeyCode)) {
        return false;
    }

    return true;
}

//
// Unregister global hotkey
//
void AutoTypePlatformWin::unregisterGlobalShortcut(Qt::Key key, Qt::KeyboardModifiers modifiers)
{
    Q_UNUSED(key);
    Q_UNUSED(modifiers);

    ::UnregisterHotKey(nullptr, HOTKEY_ID);
}

//
// Native event filter
//
int AutoTypePlatformWin::platformEventFilter(void* event)
{
    MSG *msg = static_cast<MSG *>(event);

    if (msg->message == WM_HOTKEY) {
        Q_EMIT globalShortcutTriggered();
        return 1;
    }

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
    DWORD wVk = qtToNativeKeyCode(key);
    if (wVk == 0) {
        return;
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
// see: https://msdn.microsoft.com/de-de/library/windows/desktop/dd375731%28v=vs.85%29.aspx
//
DWORD AutoTypePlatformWin::qtToNativeKeyCode(Qt::Key key)
{
    switch (key) {
    case Qt::Key_Backspace:
        return VK_BACK;     // 0x08
    case Qt::Key_Tab:
    case Qt::Key_Backtab:
        return VK_TAB;      // 0x09
    case Qt::Key_Clear:
        return VK_CLEAR;    // 0x0C
    case Qt::Key_Enter:
    case Qt::Key_Return:
        return VK_RETURN;   // 0x0D
    case Qt::Key_Pause:
        return VK_PAUSE;    // 0x13
    case Qt::Key_CapsLock:
        return VK_CAPITAL;  // 0x14
    case Qt::Key_Escape:
        return VK_ESCAPE;   // 0x1B
    case Qt::Key_Space:
        return VK_SPACE;    // 0x20
    case Qt::Key_PageUp:
        return VK_PRIOR;    // 0x21
    case Qt::Key_PageDown:
        return VK_NEXT;     // 0x22
    case Qt::Key_End:
        return VK_END;      // 0x23
    case Qt::Key_Home:
        return VK_HOME;     // 0x24
    case Qt::Key_Left:
        return VK_LEFT;     // 0x25
    case Qt::Key_Up:
        return VK_UP;       // 0x26
    case Qt::Key_Right:
        return VK_RIGHT;    // 0x27
    case Qt::Key_Down:
        return VK_DOWN;     // 0x28

    case Qt::Key_0:
        return 0x30;        // 0x30
    case Qt::Key_1:
        return 0x31;        // 0x31
    case Qt::Key_2:
        return 0x32;        // 0x32
    case Qt::Key_3:
        return 0x33;        // 0x33
    case Qt::Key_4:
        return 0x34;        // 0x34
    case Qt::Key_5:
        return 0x35;        // 0x35
    case Qt::Key_6:
        return 0x36;        // 0x36
    case Qt::Key_7:
        return 0x37;        // 0x37
    case Qt::Key_8:
        return 0x38;        // 0x38
    case Qt::Key_9:
        return 0x39;        // 0x39

    case Qt::Key_A:
        return 0x41;        // 0x41
    case Qt::Key_B:
        return 0x42;        // 0x42
    case Qt::Key_C:
        return 0x43;        // 0x43
    case Qt::Key_D:
        return 0x44;        // 0x44
    case Qt::Key_E:
        return 0x45;        // 0x45
    case Qt::Key_F:
        return 0x46;        // 0x46
    case Qt::Key_G:
        return 0x47;        // 0x47
    case Qt::Key_H:
        return 0x48;        // 0x48
    case Qt::Key_I:
        return 0x49;        // 0x49
    case Qt::Key_J:
        return 0x4A;        // 0x4A
    case Qt::Key_K:
        return 0x4B;        // 0x4B
    case Qt::Key_L:
        return 0x4C;        // 0x4C
    case Qt::Key_M:
        return 0x4D;        // 0x4D
    case Qt::Key_N:
        return 0x4E;        // 0x4E
    case Qt::Key_O:
        return 0x4F;        // 0x4F
    case Qt::Key_P:
        return 0x50;        // 0x50
    case Qt::Key_Q:
        return 0x51;        // 0x51
    case Qt::Key_R:
        return 0x52;        // 0x52
    case Qt::Key_S:
        return 0x53;        // 0x53
    case Qt::Key_T:
        return 0x54;        // 0x54
    case Qt::Key_U:
        return 0x55;        // 0x55
    case Qt::Key_V:
        return 0x56;        // 0x56
    case Qt::Key_W:
        return 0x57;        // 0x57
    case Qt::Key_X:
        return 0x58;        // 0x58
    case Qt::Key_Y:
        return 0x59;        // 0x59
    case Qt::Key_Z:
        return 0x5A;        // 0x5A

    case Qt::Key_F1:
        return VK_F1;       // 0x70
    case Qt::Key_F2:
        return VK_F2;       // 0x71
    case Qt::Key_F3:
        return VK_F3;       // 0x72
    case Qt::Key_F4:
        return VK_F4;       // 0x73
    case Qt::Key_F5:
        return VK_F5;       // 0x74
    case Qt::Key_F6:
        return VK_F6;       // 0x75
    case Qt::Key_F7:
        return VK_F7;       // 0x76
    case Qt::Key_F8:
        return VK_F8;       // 0x77
    case Qt::Key_F9:
        return VK_F9;       // 0x78
    case Qt::Key_F10:
        return VK_F10;      // 0x79
    case Qt::Key_F11:
        return VK_F11;      // 0x7A
    case Qt::Key_F12:
        return VK_F12;      // 0x7B
    case Qt::Key_F13:
        return VK_F13;      // 0x7C
    case Qt::Key_F14:
        return VK_F14;      // 0x7D
    case Qt::Key_F15:
        return VK_F15;      // 0x7E
    case Qt::Key_F16:
        return VK_F16;      // 0x7F
    case Qt::Key_F17:
        return VK_F17;      // 0x80
    case Qt::Key_F18:
        return VK_F18;      // 0x81
    case Qt::Key_F19:
        return VK_F19;      // 0x82
    case Qt::Key_F20:
        return VK_F20;      // 0x83
    case Qt::Key_F21:
        return VK_F21;      // 0x84
    case Qt::Key_F22:
        return VK_F22;      // 0x85
    case Qt::Key_F23:
        return VK_F23;      // 0x86
    case Qt::Key_F24:
        return VK_F24;      // 0x87

    case Qt::Key_NumLock:
        return VK_NUMLOCK;  // 0x90
    case Qt::Key_ScrollLock:
        return VK_SCROLL;   // 0x91
    default:
        Q_ASSERT(false);
        return 0;
    }
}

//
// Translate qt key modifiers to windows modifiers
//
DWORD AutoTypePlatformWin::qtToNativeModifiers(Qt::KeyboardModifiers modifiers)
{
    DWORD nativeModifiers = 0;

    if (modifiers & Qt::ShiftModifier) {
        nativeModifiers |= MOD_SHIFT;
    }
    if (modifiers & Qt::ControlModifier) {
        nativeModifiers |= MOD_CONTROL;
    }
    if (modifiers & Qt::AltModifier) {
        nativeModifiers |= MOD_ALT;
    }
    if (modifiers & Qt::MetaModifier) {
        nativeModifiers |= MOD_WIN;
    }

    return nativeModifiers;
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
        if (::IsWindowVisible(hwndTry)) {
            break;
        }
        hwndWalk = hwndTry;
    }

    return hwndWalk == hwnd;
}

//
// Window title enum proc
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
    int count = ::GetWindowTextW(hwnd, title, MAX_WINDOW_TITLE_LENGTH);
    QStringList *list = reinterpret_cast<QStringList *>(lParam);

    if (list != nullptr && count > 0) {
        // Add window title
        list->append(QString::fromUtf16(reinterpret_cast<const ushort *>(title), count));
    }

    return TRUE;
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
