//========================================================================
// GLFW 3.1 Win32 - www.glfw.org
//------------------------------------------------------------------------
// Copyright (c) 2002-2006 Marcus Geelnard
// Copyright (c) 2006-2010 Camilla Berglund <elmindreda@elmindreda.org>
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


static void targum(const WCHAR* lol)
{
    char* name = _glfwCreateUTF8FromWideString(lol);
    puts(name);
    free(name);
}

static BOOL CALLBACK deviceCallback(const DIDEVICEINSTANCEW* ddi, void* ref)
{
    targum(ddi->tszProductName);
    targum(ddi->tszInstanceName);
    return DIENUM_CONTINUE;
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW internal API                      //////
//////////////////////////////////////////////////////////////////////////

// Initialize joystick interface
//
void _glfwInitJoysticks(void)
{
    if (_glfw_DirectInput8Create)
    {
        if (SUCCEEDED(_glfw_DirectInput8Create(GetModuleHandle(NULL),
                                               DIRECTINPUT_VERSION,
                                               &IID_IDirectInput8W,
                                               (void**) &_glfw.win32.dinput8.object,
                                               NULL)))
        {
            IDirectInput_EnumDevices(_glfw.win32.dinput8.object,
                                     DI8DEVCLASS_GAMECTRL,
                                     deviceCallback,
                                     0,
                                     DIEDFL_ATTACHEDONLY);
        }
    }

    if (_glfw_XInputGetCapabilities)
    {
        DWORD i;

        for (i = 0;  i < 4;  i++)
        {
            XINPUT_CAPABILITIES xic;

            if (_glfw_XInputGetCapabilities(i, XINPUT_FLAG_GAMEPAD, &xic) !=
                ERROR_SUCCESS)
            {
                continue;
            }

            _glfw.win32_js[i].axisCount = 6;
            _glfw.win32_js[i].buttonCount = 14;
            _glfw.win32_js[i].name = strdup("Quux");
            _glfw.win32_js[i].present = GL_TRUE;
        }
    }
}

// Close all opened joystick handles
//
void _glfwTerminateJoysticks(void)
{
    DWORD i;

    for (i = 0;  i < 4;  i++)
        free(_glfw.win32_js[i].name);
}


//////////////////////////////////////////////////////////////////////////
//////                       GLFW platform API                      //////
//////////////////////////////////////////////////////////////////////////

int _glfwPlatformJoystickPresent(int joy)
{
    return _glfw.win32_js[joy].present;
}

const float* _glfwPlatformGetJoystickAxes(int joy, int* count)
{
    XINPUT_STATE xis;

    if (_glfw_XInputGetState(joy, &xis) != ERROR_SUCCESS)
        return NULL;

    _glfw.win32_js[joy].axes[0] = (xis.Gamepad.sThumbLX + 0.5f) / 32767.f;
    _glfw.win32_js[joy].axes[1] = (xis.Gamepad.sThumbLY + 0.5f) / 32767.f;
    _glfw.win32_js[joy].axes[2] = (xis.Gamepad.sThumbRX + 0.5f) / 32767.f;
    _glfw.win32_js[joy].axes[3] = (xis.Gamepad.sThumbRY + 0.5f) / 32767.f;
    _glfw.win32_js[joy].axes[4] = xis.Gamepad.bLeftTrigger / 127.5f - 1.f;
    _glfw.win32_js[joy].axes[5] = xis.Gamepad.bRightTrigger / 127.5f - 1.f;

    *count = 6;
    return _glfw.win32_js[joy].axes;
}

const unsigned char* _glfwPlatformGetJoystickButtons(int joy, int* count)
{
    XINPUT_STATE xis;

    if (_glfw_XInputGetState(joy, &xis) != ERROR_SUCCESS)
        return NULL;

    _glfw.win32_js[joy].buttons[0] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 1 : 0;
    _glfw.win32_js[joy].buttons[1] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 1 : 0;
    _glfw.win32_js[joy].buttons[2] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? 1 : 0;
    _glfw.win32_js[joy].buttons[3] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? 1 : 0;
    _glfw.win32_js[joy].buttons[4] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1 : 0;
    _glfw.win32_js[joy].buttons[5] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 1 : 0;
    _glfw.win32_js[joy].buttons[6] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? 1 : 0;
    _glfw.win32_js[joy].buttons[7] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? 1 : 0;
    _glfw.win32_js[joy].buttons[8] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1 : 0;
    _glfw.win32_js[joy].buttons[9] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1 : 0;
    _glfw.win32_js[joy].buttons[10] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 1 : 0;
    _glfw.win32_js[joy].buttons[11] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1 : 0;
    _glfw.win32_js[joy].buttons[12] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 1 : 0;
    _glfw.win32_js[joy].buttons[13] = (xis.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 1 : 0;

    *count = 14;
    return _glfw.win32_js[joy].buttons;
}

const char* _glfwPlatformGetJoystickName(int joy)
{
    return _glfw.win32_js[joy].name;
}

