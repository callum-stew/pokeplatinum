#include "macros/scrcmd.inc"
#include "res/text/bank/lake_acuity_low_water.h"

    .data

    ScriptEntry _0006
    ScriptEntryEnd

_0006:
    PlayFanfare SEQ_SE_CONFIRM
    LockAll
    FacePlayer
    GoToIfSet 169, _002B
    SetFlag 169
    BufferRivalName 0
    Message 0
    WaitABXPadPress
    CloseMessage
    ReleaseAll
    End

_002B:
    BufferRivalName 0
    Message 1
    WaitABXPadPress
    CloseMessage
    ReleaseAll
    End

    .byte 0
    .byte 0
    .byte 0
