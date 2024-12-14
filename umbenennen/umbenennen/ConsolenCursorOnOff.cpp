#include "ConsolenCursorOnOff.h"

#include <windows.h>

namespace ConsolenCursor
{

    bool OnOff(bool on)
    {

        auto hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

        CONSOLE_CURSOR_INFO curInfo{};
        GetConsoleCursorInfo(hStdOut, &curInfo);
        if( !!curInfo.bVisible == on )
            return false;//keine änderung
        curInfo.bVisible = on;
        SetConsoleCursorInfo(hStdOut, &curInfo);
        return true;//änderung
    }
}
