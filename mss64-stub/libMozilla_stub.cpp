// Stand-alone stub for libMozilla. The real source pulls in the SOE-era
// XULRunner/Gecko build and won't compile under modern x64 toolchains;
// since the in-game embedded browser is dead in this fork, we provide
// no-op implementations against the public header so the link succeeds.
//
// All Window methods are no-ops; createWindow returns null.

#include "../src/external/3rd/library/libMozilla/src/win32/libMozilla.h"

namespace libMozilla
{
    bool init(void*, const char*)             { return true; }
    void update()                             {}
    void release()                            {}

    void enableMemoryCache(bool)              {}
    void enableDiskCache(bool, unsigned)      {}
    void setUserAgent(const char*)            {}

    Window *createWindow(unsigned, unsigned)  { return 0; }
    void    destroyWindow(Window*)            {}

    void           Window::setSize(unsigned, unsigned)                              {}
    void           Window::setFocus(bool)                                           {}
    void           Window::setCallback(ICallback*)                                  {}
    void           Window::setRenderOnComplete(bool)                                {}
    float          Window::getProgress(bool &bLoading)                              { bLoading = false; return 0.0f; }
    const wchar_t *Window::getStatus() const                                        { return L""; }
    const char    *Window::getURI() const                                           { return ""; }
    bool           Window::getCaret(int&, int&, int&, int&)                         { return false; }
    void           Window::navigateTo(const wchar_t*, const char*, const unsigned)  {}
    void           Window::navigateStop()                                           {}
    bool           Window::canNavigateBack()                                        { return false; }
    void           Window::navigateBack()                                           {}
    bool           Window::canNavigateForward()                                     { return false; }
    void           Window::navigateForward()                                        {}
    void           Window::reload()                                                 {}
    void           Window::onLeftMouseDown(int, int, unsigned)                      {}
    void           Window::onLeftMouseUp(int, int, unsigned)                        {}
    void           Window::onMiddleMouseDown(int, int, unsigned)                    {}
    void           Window::onMiddleMouseUp(int, int, unsigned)                      {}
    void           Window::onRightMouseDown(int, int, unsigned)                     {}
    void           Window::onRightMouseUp(int, int, unsigned)                       {}
    void           Window::onMouseMove(int, int, unsigned)                          {}
    void           Window::onMouseWheel(int, unsigned)                              {}
    void           Window::onKeyPress(int, int, unsigned)                           {}
    void           Window::onCommand(Command)                                       {}
    bool           Window::render(IBlitter*)                                        { return false; }

    Window::Window()  : m_pImpl(0) {}
    Window::~Window()              {}
}
