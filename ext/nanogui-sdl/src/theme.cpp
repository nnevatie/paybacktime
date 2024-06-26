/*
    src/theme.cpp -- Storage class for basic theme-related properties

    The text box widget was contributed by Christian Schueller.

    NanoGUI was developed by Wenzel Jakob <wenzel@inf.ethz.ch>.
    The widget drawing code is based on the NanoVG demo application
    by Mikko Mononen.

    All rights reserved. Use of this source code is governed by a
    BSD-style license that can be found in the LICENSE.txt file.
*/

#include <nanogui/theme.h>
#include <nanogui/opengl.h>
#include <nanogui/nanogui_resources.h>

NAMESPACE_BEGIN(nanogui)

namespace
{

nanogui::Color color(int luma, int alpha)
{
    Eigen::Vector3f base(255.f, 230.f, 210.f);
    return nanogui::Color((base * (luma / 255.f)).cast<int>(), alpha);
}

}

Theme::Theme(NVGcontext *ctx) {
    mStandardFontSize                 = 18;
    mButtonFontSize                   = 18;
    mTextBoxFontSize                  = 18;
    mWindowCornerRadius               = 2;
    mWindowHeaderHeight               = 30;
    mWindowDropShadowSize             = 10;
    mButtonCornerRadius               = 2;

    mTabBorderWidth                   = 0.75f;
    mTabInnerMargin                   = 5;
    mTabMinButtonWidth                = 20;
    mTabMaxButtonWidth                = 160;
    mTabControlWidth                  = 20;
    mTabButtonHorizontalPadding       = 10;
    mTabButtonVerticalPadding         = 2;

    mDropShadow                       = color(0, 128);
    mTransparent                      = color(0, 0);
    mBorderDark                       = color(29, 255);
    mBorderLight                      = color(92, 255);
    mBorderMedium                     = color(35, 255);
    mTextColor                        = color(255, 160);
    mDisabledTextColor                = color(255, 80);
    mTextColorShadow                  = color(0, 160);
    mIconColor                        = mTextColor;

    mButtonGradientTopFocused         = color(64, 255);
    mButtonGradientBotFocused         = color(48, 255);
    mButtonGradientTopUnfocused       = color(74, 255);
    mButtonGradientBotUnfocused       = color(58, 255);
    mButtonGradientTopPushed          = color(41, 255);
    mButtonGradientBotPushed          = color(29, 255);

    /* Window-related */
    mWindowFillUnfocused              = color(43, 230);
    mWindowFillFocused                = color(45, 230);
    mWindowTitleUnfocused             = color(220, 160);
    mWindowTitleFocused               = color(255, 190);

    mWindowHeaderGradientTop          = mButtonGradientTopUnfocused;
    mWindowHeaderGradientBot          = mButtonGradientBotUnfocused;
    mWindowHeaderSepTop               = mBorderLight;
    mWindowHeaderSepBot               = mBorderDark;

    mWindowPopup                      = color(50, 255);
    mWindowPopupTransparent           = color(50, 0);

    mFontNormal = nvgCreateFont(ctx, "sans",      "data/play_regular.ttf");
    mFontBold   = nvgCreateFont(ctx, "sans-bold", "data/play_bold.ttf");

    #if 0
    mFontNormal = nvgCreateFontMem(ctx, "sans", roboto_regular_ttf,
                                   roboto_regular_ttf_size, 0);
    mFontBold = nvgCreateFontMem(ctx, "sans-bold", roboto_bold_ttf,
                                 roboto_bold_ttf_size, 0);
    #endif
    mFontIcons = nvgCreateFontMem(ctx, "icons", entypo_ttf,
                                  entypo_ttf_size, 0);
    if (mFontNormal == -1 || mFontBold == -1 || mFontIcons == -1)
        throw std::runtime_error("Could not load fonts!");
}

NAMESPACE_END(nanogui)
