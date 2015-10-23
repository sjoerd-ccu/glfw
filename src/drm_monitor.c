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

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

struct _GLFWvidmodeDRM; {
    GLFWvidmode     base;
    uint32_t        flags;
};

#if 0
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

struct drm_edid {
    char eisa_id[13];
    char monitor_name[13];
    char pnp_id[5];
    char serial_number[13];
};

static void
edid_parse_string(const uint8_t *data, char text[])
{
    int i;
    int replaced = 0;

    /* this is always 12 bytes, but we can't guarantee it's null
     * terminated or not junk. */
    strncpy(text, (const char *) data, 12);

    /* remove insane chars */
    for (i = 0; text[i] != '\0'; i++)
    {
        if (text[i] == '\n' || text[i] == '\r')
        {
            text[i] = '\0';
            break;
        }
    }

    /* ensure string is printable */
    for (i = 0; text[i] != '\0'; i++)
    {
        if (!isprint(text[i]))
        {
            text[i] = '-';
            replaced++;
        }
    }

    /* if the string is random junk, ignore the string */
    if (replaced > 4)
        text[0] = '\0';
}

#define EDID_DESCRIPTOR_ALPHANUMERIC_DATA_STRING        0xfe
#define EDID_DESCRIPTOR_DISPLAY_PRODUCT_NAME            0xfc
#define EDID_DESCRIPTOR_DISPLAY_PRODUCT_SERIAL_NUMBER   0xff
#define EDID_OFFSET_DATA_BLOCKS                         0x36
#define EDID_OFFSET_LAST_BLOCK                          0x6c
#define EDID_OFFSET_PNPID                               0x08
#define EDID_OFFSET_SERIAL                              0x0c

static int
edid_parse(struct drm_edid *edid, const uint8_t *data, size_t length)
{
    int i;
    uint32_t serial_number;

    /* check header */
    if (length < 128)
        return -1;
    if (data[0] != 0x00 || data[1] != 0xff)
        return -1;

    /* decode the PNP ID from three 5 bit words packed into 2 bytes
     * /--08--\/--09--\
     * 7654321076543210
     * |\---/\---/\---/
     * R  C1   C2   C3 */
    edid->pnp_id[0] = 'A' + ((data[EDID_OFFSET_PNPID + 0] & 0x7c) / 4) - 1;
    edid->pnp_id[1] = 'A' + ((data[EDID_OFFSET_PNPID + 0] & 0x3) * 8) + ((data[EDID_OFFSET_PNPID + 1] & 0xe0) / 32) - 1;
    edid->pnp_id[2] = 'A' + (data[EDID_OFFSET_PNPID + 1] & 0x1f) - 1;
    edid->pnp_id[3] = '\0';

    /* maybe there isn't a ASCII serial number descriptor, so use this instead */
    serial_number = (uint32_t) data[EDID_OFFSET_SERIAL + 0];
    serial_number += (uint32_t) data[EDID_OFFSET_SERIAL + 1] * 0x100;
    serial_number += (uint32_t) data[EDID_OFFSET_SERIAL + 2] * 0x10000;
    serial_number += (uint32_t) data[EDID_OFFSET_SERIAL + 3] * 0x1000000;
    if (serial_number > 0)
        sprintf(edid->serial_number, "%lu", (unsigned long) serial_number);

    /* parse EDID data */
    for (i = EDID_OFFSET_DATA_BLOCKS;
         i <= EDID_OFFSET_LAST_BLOCK;
         i += 18)
    {
        /* ignore pixel clock data */
        if (data[i] != 0)
            continue;
        if (data[i+2] != 0)
            continue;

        /* any useful blocks? */
        if (data[i+3] == EDID_DESCRIPTOR_DISPLAY_PRODUCT_NAME)
            edid_parse_string(&data[i+5], edid->monitor_name);
        else if (data[i+3] == EDID_DESCRIPTOR_DISPLAY_PRODUCT_SERIAL_NUMBER)
            edid_parse_string(&data[i+5], edid->serial_number);
        else if (data[i+3] == EDID_DESCRIPTOR_ALPHANUMERIC_DATA_STRING)
            edid_parse_string(&data[i+5], edid->eisa_id);
    }
    return 0;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

void _glfwAddOutput(drmModeRes *resources, drmModeConnector *connector)
{
    _GLFWmonitor* monitor;
    char *name;
    drmModeEncoder* encoder = NULL;
    drmModeModeInfo* selected_mode = NULL;
    int i, ret;

    for (i = 0; i < connector->count_props; ++i)
    {
        drmModePropertyRes* property = drmModeGetProperty(_glfw.drm.modeset_fd, connector->props[i]);
        if (!strcmp(property->name, "EDID"))
        {
            drmModePropertyBlobRes* edid_blob = drmModeGetPropertyBlob(_glfw.drm.modeset_fd, connector->prop_values[i]);
            struct drm_edid edid;
            ret = edid_parse(&edid, edid_blob->data, edid_blob->length);
            if (!ret)
            {
                /* XXX: Use the other values as well? */
                name = strdup(edid.monitor_name);
            }
            drmModeFreePropertyBlob(edid_blob);
        }
        printf("property: %s %lu\n", property->name, connector->prop_values[i]);
        drmModeFreeProperty(property);
    }

    monitor = _glfwAllocMonitor(name, 0, 0);

#if 0
    monitor->drm.modes = calloc(4, sizeof(_GLFWvidmodeDRM));
    monitor->drm.modesSize = 4;

    monitor->drm.output = output;
    wl_output_add_listener(output, &output_listener, monitor);
#endif

    /* find highest resolution mode: */
    for (i = 0; i < connector->count_modes; i++)
    {
        drmModeModeInfo* mode = &connector->modes[i];
        if (mode->type & DRM_MODE_TYPE_PREFERRED)
        {
            selected_mode = mode;
            break;
        }
    }

    if (!selected_mode)
    {
        _glfwInputError(GLFW_PLATFORM_ERROR, "DRM: No preferred mode");
        return;
    }

    printf("%d: %d×%d (0x%x, 0x%x)\n", i, selected_mode->hdisplay, selected_mode->vdisplay, selected_mode->flags, selected_mode->type);

    /* find encoder: */
    for (i = 0; i < resources->count_encoders; i++) {
        encoder = drmModeGetEncoder(_glfw.drm.modeset_fd, resources->encoders[i]);
        printf("%d %d\n", encoder->encoder_id, connector->encoder_id);
        if (encoder->encoder_id == connector->encoder_id)
            break;
        drmModeFreeEncoder(encoder);
        encoder = NULL;
    }

    if (!encoder) {
        _glfwInputError(GLFW_PLATFORM_ERROR, "DRM: No encoder");
        return;
    }

    _glfw.drm.crtc_id = encoder->crtc_id;
    _glfw.drm.connector_id = connector->connector_id;

    if (_glfw.drm.monitorsCount + 1 >= _glfw.drm.monitorsSize)
    {
        _GLFWmonitor** monitors = _glfw.drm.monitors;
        int size = _glfw.drm.monitorsSize * 2;

        monitors = realloc(monitors, size * sizeof(_GLFWmonitor*));

        _glfw.drm.monitors = monitors;
        _glfw.drm.monitorsSize = size;
    }

    _glfw.drm.monitors[_glfw.drm.monitorsCount++] = monitor;
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

