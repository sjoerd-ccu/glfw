//========================================================================
// GLFW 3.2 DRM - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2015 Emmanuel Gil Peyrot <linkmauve@linkmauve.fr>
//
// This software is provided 'as-is', without any express or implied
// warranty. In no event will the authors be held liable for any damages
// arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would
//    be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such, and must not
//    be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source
//    distribution.
//
//========================================================================

#ifndef _glfw3_drm_platform_h_
#define _glfw3_drm_platform_h_

#include <xf86drm.h>
#include <xf86drmMode.h>
#include <gbm.h>
//#include <xkbcommon/xkbcommon.h>

#include "posix_tls.h"
#include "posix_time.h"
#include "linux_joystick.h"
//#include "xkb_unicode.h"

#if defined(_GLFW_EGL)
 #include "egl_context.h"
#else
 #error "The DRM backend depends on EGL platform support"
#endif

#define _GLFW_EGL_NATIVE_WINDOW         window->drm.surface
#define _GLFW_EGL_NATIVE_DISPLAY        _glfw.drm.gbm_device

#define _GLFW_PLATFORM_WINDOW_STATE         _GLFWwindowDRM  drm
#define _GLFW_PLATFORM_LIBRARY_WINDOW_STATE _GLFWlibraryDRM drm
#define _GLFW_PLATFORM_MONITOR_STATE        _GLFWmonitorDRM drm
#define _GLFW_PLATFORM_CURSOR_STATE         _GLFWcursorDRM  drm


// DRM-specific video mode data
//
typedef struct _GLFWvidmodeDRM _GLFWvidmodeDRM;


// DRM-specific per-window data
//
typedef struct _GLFWwindowDRM
{
    int                         width, height;
    GLFWbool                    visible;
    struct gbm_surface*         surface;
    _GLFWcursor*                currentCursor;
    double                      cursorPosX, cursorPosY;
} _GLFWwindowDRM;


// DRM-specific global data
//
typedef struct _GLFWlibraryDRM
{
    int                         fd;
    struct gbm_device*          gbm_device;
    uint32_t                    pointerSerial;

#ifndef USE_RENDER_NODES
    int                         modeset_fd;
    drmModeModeInfo*            mode;
    uint32_t                    crtc_id;
    uint32_t                    connector_id;
#endif

    _GLFWmonitor**              monitors;
    int                         monitorsCount;
    int                         monitorsSize;

#if 0
    struct {
        struct xkb_context*     context;
        struct xkb_keymap*      keymap;
        struct xkb_state*       state;
        xkb_mod_mask_t          control_mask;
        xkb_mod_mask_t          alt_mask;
        xkb_mod_mask_t          shift_mask;
        xkb_mod_mask_t          super_mask;
        unsigned int            modifiers;
    } xkb;
#endif

    _GLFWwindow*                pointerFocus;
    _GLFWwindow*                keyboardFocus;

} _GLFWlibraryDRM;


// DRM-specific per-monitor data
//
typedef struct _GLFWmonitorDRM
{
    _GLFWvidmodeDRM*        modes;
    int                     modesCount;
    int                     modesSize;
    _GLFWvidmodeDRM*        current_mode;

    uint32_t                connector_id;
    uint32_t                crtc_id;

} _GLFWmonitorDRM;


// DRM-specific per-cursor data
//
typedef struct _GLFWcursorDRM
{
#if 0
    struct wl_buffer*           buffer;
#endif
    int                         width, height;
    int                         xhot, yhot;
} _GLFWcursorDRM;

void _glfwAddOutput(drmModeRes *resources, drmModeConnector *connector);


#endif // _glfw3_drm_platform_h_
