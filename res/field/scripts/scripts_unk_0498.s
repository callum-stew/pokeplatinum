#include "macros/scrcmd.inc"
#include "res/text/bank/unk_0539.h"

    .data

    ScriptEntry _0006
    ScriptEntryEnd

_0006:
    SetVar 0x800C, 0
    ScrCmd_20A 0x800C
    SetVar 0x8008, 0x800C
    GoToIfEq 0x8008, 0, _0043
    GoToIfEq 0x8008, 1, _0047
    GoToIfEq 0x8008, 2, _0050
    GoTo _0059

_0043:
    ReleaseAll
    End

_0047:
    Message 0
    GoTo _0068

_0050:
    Message 1
    GoTo _0068

_0059:
    WaitTime 30, 0x800C
    Message 2
    GoTo _0068

_0068:
    WaitABXPadPress
    CloseMessage
    ReleaseAll
    End
