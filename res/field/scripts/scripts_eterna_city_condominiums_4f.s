#include "macros/scrcmd.inc"
#include "res/text/bank/eterna_city_condominiums_4f.h"

    .data

    ScriptEntry _0006
    ScriptEntryEnd

_0006:
    PlayFanfare SEQ_SE_CONFIRM
    LockAll
    FacePlayer
    GoToIfSet 118, _004B
    Message 0
    SetVar 0x8004, 0x14B
    SetVar 0x8005, 1
    ScrCmd_07D 0x8004, 0x8005, 0x800C
    GoToIfEq 0x800C, 0, _0056
    SetFlag 118
    CallCommonScript 0x7E0
    CloseMessage
    ReleaseAll
    End

_004B:
    Message 1
    WaitABXPadPress
    CloseMessage
    ReleaseAll
    End

_0056:
    CallCommonScript 0x7E1
    CloseMessage
    ReleaseAll
    End
