#ifndef POKEPLATINUM_STRUCT_02095E80_T_H
#define POKEPLATINUM_STRUCT_02095E80_T_H

#include <nitro/fx/fx.h>
#include <nnsys.h>

#include "struct_decls/struct_0200112C_decl.h"
#include "struct_decls/struct_02001AF4_decl.h"
#include "struct_decls/struct_02027F8C_decl.h"
#include "struct_decls/struct_0202B370_decl.h"
#include "struct_decls/struct_02095E80_decl.h"
#include "struct_decls/struct_party_decl.h"
#include "struct_defs/archived_sprite.h"
#include "struct_defs/chatot_cry.h"
#include "struct_defs/pokemon_summary.h"
#include "struct_defs/struct_0200C738.h"
#include "struct_defs/struct_02027F8C.h"
#include "struct_defs/struct_02095E80_sub1.h"

#include "field/field_system_decl.h"
#include "overlay088/struct_ov88_0223C370.h"
#include "overlay088/struct_ov88_0223C8AC.h"

#include "bg_window.h"
#include "cell_actor.h"
#include "message.h"
#include "overlay_manager.h"
#include "savedata.h"
#include "sprite_resource.h"
#include "strbuf.h"
#include "string_list.h"
#include "string_template.h"
#include "sys_task_manager.h"

struct UnkStruct_02095E80_t {
    FieldSystem *fieldSystem;
    SaveData *unk_04;
    UnkStruct_ov88_0223C370 *unk_08;
    PokemonSummary unk_0C;
    int unk_3C;
    OverlayManager *unk_40;
    int unk_44;
    int unk_48;
    int unk_4C;
    int unk_50;
    int unk_54;
    int unk_58;
    int unk_5C;
    int unk_60[2];
    int unk_68;
    BOOL unk_6C;
    UnkStruct_02095E80_sub1 unk_70;
    int unk_88[2];
    u16 unk_90;
    u16 unk_92[5][17];
    u16 unk_13C;
    u16 unk_13E;
    int unk_140;
    u16 unk_144[3];
    u32 unk_14C[2];
    u32 unk_154[2][3];
    int unk_16C;
    int unk_170;
    BgConfig *unk_174;
    StringTemplate *unk_178;
    StringTemplate *unk_17C;
    StringTemplate *unk_180;
    MessageLoader *unk_184;
    Strbuf *unk_188;
    Strbuf *unk_18C;
    Strbuf *unk_190;
    CellActorCollection *unk_194;
    UnkStruct_0200C738 unk_198;
    SpriteResourceCollection *unk_324[4];
    SpriteResource *unk_334[2][4];
    CellActorResourceData unk_354;
    CellActorResourceData unk_378;
    CellActor *unk_39C[14];
    CellActor *unk_3D4[12];
    CellActor *unk_404[12];
    CellActor *unk_434[12];
    CellActor *unk_464[6];
    CellActor *unk_47C;
    SysTask *unk_480[7];
    Window unk_49C[34];
    UIControlData *unk_6BC;
    StringList *unk_6C0;
    UIControlData *unk_6C4;
    int unk_6C8;
    int unk_6CC;
    int unk_6D0;
    int unk_6D4;
    VecFx32 unk_6D8[2];
    int unk_6F0;
    UnkStruct_ov88_0223C8AC unk_6F4[13];
    int unk_7B8;
    int unk_7BC;
    int unk_7C0;
    int unk_7C4[7];
    void *unk_7E0;
    NNSG2dCharacterData *unk_7E4;
    void *unk_7E8[12];
    NNSG2dCharacterData *unk_818[12];
    u8 unk_848[2][3200];
    ArchivedSprite unk_2148[2];
    int unk_2168;
    u8 unk_216C[256];
    int (*unk_226C)(UnkStruct_02095E80 *);
    Party *unk_2270;
    Party *unk_2274;
    ChatotCry *unk_2278;
    UnkStruct_02027F8C *unk_227C;
    UnkStruct_02027F8C unk_2280;
    int unk_2308;
    int unk_230C;
    int unk_2310;
    int unk_2314;
    int unk_2318;
    VecFx32 unk_231C[2];
    VecFx32 unk_2334[2];
    u8 unk_234C[2][1424];
    u8 unk_2E6C[2][1004];
    int unk_3644[32];
    int unk_36C4;
    int unk_36C8;
    StringTemplate *unk_36CC;
    MessageLoader *unk_36D0;
    StringList *unk_36D4;
    BmpList *unk_36D8;
    Window unk_36DC;
    UnkStruct_0202B370 *unk_36EC;
    int unk_36F0;
    int unk_36F4;
    int unk_36F8;
    int unk_36FC;
    void *unk_3700;
    int unk_3704;
    int unk_3708;
};

#endif // POKEPLATINUM_STRUCT_02095E80_T_H
