/*
    nanogui/opengl.h -- Pulls in OpenGL, GLEW (if needed), GLFW, and
    NanoVG header files

    NanoGUI was developed by Wenzel Jakob <wenzel@inf.ethz.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#ifndef __NGSDL_OPENGL_INCLUDE_H__
#define __NGSDL_OPENGL_INCLUDE_H__

#include <SDL_opengl.h>
#include <nanogui/common.h>
#include <nanovg.h>

NAMESPACE_BEGIN(nanogui)

/// Determine whether an icon ID is a texture loaded via nvgImageIcon
inline bool nvgIsImageIcon(int value) { return value < 1024; }

/// Determine whether an icon ID is a font-based icon (e.g. from the entypo.ttf font)
inline bool nvgIsFontIcon(int value) { return value >= 1024; }


NAMESPACE_END(nanogui)

#endif
