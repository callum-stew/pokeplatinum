#include "macros/scrcmd.inc"

    .data

    ScriptEntry _0006
    ScriptEntryEnd

_0006:
    Call _0014
    ScrCmd_285 0x410B, 0x410C
    End

_0014:
    AddVar 0x410C, 1
    Return
