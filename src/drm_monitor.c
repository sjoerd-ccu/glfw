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

#include "internal.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>


#if 0
struct _GLFWvidmodeDRM
{
    GLFWvidmode         base;
    uint32_t            flags;
};

static void geometry(void* data,
                     struct wl_output* output,
                     int32_t x,
                     int32_t y,
                     int32_t physicalWidth,
                     int32_t physicalHeight,
                     int32_t subpixel,
                     const char* make,
                     const char* model,
                     int32_t transform)
{
    struct _GLFWmonitor *monitor = data;

    monitor->drm.x = x;
    monitor->drm.y = y;
    monitor->widthMM = physicalWidth;
    monitor->heightMM = physicalHeight;
}

static void mode(void* data,
                 struct wl_output* output,
                 uint32_t flags,
                 int32_t width,
                 int32_t height,
                 int32_t refresh)
{
    struct _GLFWmonitor *monitor = data;
    _GLFWvidmodeDRM mode = { { 0 }, };

    mode.base.width = width;
    mode.base.height = height;
    mode.base.refreshRate = refresh;
    mode.flags = flags;

    if (monitor->drm.modesCount + 1 >= monitor->drm.modesSize)
    {
        int size = monitor->drm.modesSize * 2;
        _GLFWvidmodeDRM* modes =
            realloc(monitor->drm.modes,
                    size * sizeof(_GLFWvidmodeDRM));
        monitor->drm.modes = modes;
        monitor->drm.modesSize = size;
    }

    monitor->drm.modes[monitor->drm.modesCount++] = mode;
}

static void done(void* data,
                 struct wl_output* output)
{
    struct _GLFWmonitor *monitor = data;

    monitor->drm.done = GLFW_TRUE;
}

static void scale(void* data,
                  struct wl_output* output,
                  int32_t factor)
{
}

static const struct wl_output_listener output_listener = {
    geometry,
    mode,
    done,
    scale,
};
#endif


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwAddOutput(uint32_t name, uint32_t version)
{
#if 0
    _GLFWmonitor *monitor;
    struct wl_output *output;
    char name_str[80];

    memset(name_str, 0, 80 * sizeof(char));
    snprintf(name_str, 79, "wl_output@%u", name);

    if (version < 2)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR,
                        "DRM: Unsupported output interface version");
        return;
    }

    monitor = _glfwAllocMonitor(name_str, 0, 0);

    output = wl_registry_bind(_glfw.wl.registry,
                              name,
                              &wl_output_interface,
                              2);
    if (!output)
    {
        _glfwFreeMonitor(monitor);
        return;
    }

    monitor->drm.modes = calloc(4, sizeof(_GLFWvidmodeDRM));
    monitor->drm.modesSize = 4;

    monitor->drm.output = output;
    wl_output_add_listener(output, &output_listener, monitor);

    if (_glfw.wl.monitorsCount + 1 >= _glfw.wl.monitorsSize)
    {
        _GLFWmonitor** monitors = _glfw.wl.monitors;
        int size = _glfw.wl.monitorsSize * 2;

        monitors = realloc(monitors, size * sizeof(_GLFWmonitor*));

        _glfw.wl.monitors = monitors;
        _glfw.wl.monitorsSize = size;
    }

    _glfw.wl.monitors[_glfw.wl.monitorsCount++] = monitor;
#endif
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

_GLFWmonitor** _glfwPlatformGetMonitors(int* count)
{
    _GLFWmonitor** monitors;
    _GLFWmonitor* monitor;
    int i, monitorsCount = _glfw.drm.monitorsCount;

    if (_glfw.drm.monitorsCount == 0)
        goto err;

    monitors = calloc(monitorsCount, sizeof(_GLFWmonitor*));

    for (i = 0; i < monitorsCount; i++)
    {
        _GLFWmonitor* origMonitor = _glfw.drm.monitors[i];
        monitor = calloc(1, sizeof(_GLFWmonitor));

        monitor->modes =
            _glfwPlatformGetVideoModes(origMonitor,
                                       &origMonitor->drm.modesCount);
        *monitor = *_glfw.drm.monitors[i];
        monitors[i] = monitor;
    }

    *count = monitorsCount;
    return monitors;

err:
    *count = 0;
    return NULL;
}

GLFWbool _glfwPlatformIsSameMonitor(_GLFWmonitor* first, _GLFWmonitor* second)
{
    //return first->drm.output == second->drm.output;
    return GLFW_TRUE;
}

void _glfwPlatformGetMonitorPos(_GLFWmonitor* monitor, int* xpos, int* ypos)
{
    if (xpos)
        *xpos = monitor->drm.x;
    if (ypos)
        *ypos = monitor->drm.y;
}

GLFWvidmode* _glfwPlatformGetVideoModes(_GLFWmonitor* monitor, int* found)
{
#if 0
    GLFWvidmode *modes;
    int i, modesCount = monitor->drm.modesCount;

    modes = calloc(modesCount, sizeof(GLFWvidmode));

    for (i = 0;  i < modesCount;  i++)
        modes[i] = monitor->drm.modes[i].base;

    *found = modesCount;
    return modes;
#endif
    return NULL;
}

void _glfwPlatformGetVideoMode(_GLFWmonitor* monitor, GLFWvidmode* mode)
{
#if 0
    int i;

    for (i = 0;  i < monitor->drm.modesCount;  i++)
    {
        if (monitor->drm.modes[i].flags & WL_OUTPUT_MODE_CURRENT)
        {
            *mode = monitor->drm.modes[i].base;
            return;
        }
    }
#endif
}

void _glfwPlatformGetGammaRamp(_GLFWmonitor* monitor, GLFWgammaramp* ramp)
{
    // TODO
    fprintf(stderr, "_glfwPlatformGetGammaRamp not implemented yet\n");
}

void _glfwPlatformSetGammaRamp(_GLFWmonitor* monitor, const GLFWgammaramp* ramp)
{
    // TODO
    fprintf(stderr, "_glfwPlatformSetGammaRamp not implemented yet\n");
}

