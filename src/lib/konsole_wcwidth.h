// SPDX-FileCopyrightText: Markus Kuhn
// SPDX-License-Identifier: CC0-1.0

#pragma once

#include <QStringView>

int konsole_wcwidth(wchar_t ucs);

inline int konsole_wcwidth(QChar ucs)
{
    return konsole_wcwidth(static_cast<wchar_t>(ucs.unicode()));
}

// single byte char: +1, multi byte char: +2
inline int string_width(QStringView str)
{
    int w = 0;
    for (auto c : str) {
        w += konsole_wcwidth(c);
    }
    return w;
}
