#include "unk_02054884.h"

#include <nitro.h>
#include <string.h>

#include "constants/battle/condition.h"
#include "constants/items.h"

#include "struct_decls/struct_party_decl.h"

#include "overlay005/ov5_021E622C.h"

#include "heap.h"
#include "party.h"
#include "pokemon.h"
#include "save_player.h"
#include "savedata.h"
#include "trainer_info.h"
#include "unk_02017038.h"
#include "unk_0202F180.h"

BOOL Pokemon_CanBattle(Pokemon *mon)
{
    // this can be simplified further, but it won't match
    if (Pokemon_GetValue(mon, MON_DATA_CURRENT_HP, NULL) == 0) {
        return FALSE;
    }

    return !Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL);
}

BOOL sub_020548B0(int param0, SaveData *param1, u16 param2, u8 param3, u16 param4, int param5, int param6)
{
    BOOL result;
    Pokemon *mon;
    u32 v2;
    Party *party;
    TrainerInfo *trainerInfo;

    trainerInfo = SaveData_GetTrainerInfo(param1);
    party = Party_GetFromSavedata(param1);
    mon = Pokemon_New(param0);

    Pokemon_Init(mon);
    Pokemon_InitWith(mon, param2, param3, 32, FALSE, 0, OTID_NOT_SET, 0);
    Pokemon_SetCatchData(mon, trainerInfo, ITEM_POKE_BALL, param5, param6, param0);

    v2 = param4;
    Pokemon_SetValue(mon, MON_DATA_HELD_ITEM, &v2);
    result = Party_AddPokemon(party, mon);

    if (result) {
        sub_0202F180(param1, mon);
    }

    Heap_FreeToHeap(mon);

    return result;
}

BOOL sub_02054930(int param0, SaveData *param1, u16 param2, u8 param3, int param4, int param5)
{
    int v0;
    BOOL result;
    TrainerInfo *trainerInfo = SaveData_GetTrainerInfo(param1);
    Party *party = Party_GetFromSavedata(param1);
    Pokemon *mon = Pokemon_New(32);

    Pokemon_Init(mon);

    v0 = sub_02017070(param4, param5);
    ov5_021E6CF0(mon, param2, param3, trainerInfo, 4, v0);

    result = Party_AddPokemon(party, mon);
    Heap_FreeToHeap(mon);

    return result;
}

void sub_02054988(Party *party, int param1, int param2, u16 param3)
{
    Pokemon_ResetMoveSlot(Party_GetPokemonBySlotIndex(party, param1), param3, param2);
}

// In many of the functions below, C99-style iterator declaration doesn't match

int Party_HasMonWithMove(Party *party, u16 moveID)
{
    int i;
    int partyCount = Party_GetCurrentCount(party);

    for (i = 0; i < partyCount; i++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(party, i);

        if (Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) != FALSE) {
            continue;
        }

        if (Pokemon_GetValue(mon, MON_DATA_MOVE1, NULL) == moveID
            || Pokemon_GetValue(mon, MON_DATA_MOVE2, NULL) == moveID
            || Pokemon_GetValue(mon, MON_DATA_MOVE3, NULL) == moveID
            || Pokemon_GetValue(mon, MON_DATA_MOVE4, NULL) == moveID) {
            return i;
        }
    }

    return PARTY_SLOT_NONE;
}

int Party_AliveMonsCount(const Party *party)
{
    int i;
    int partyCount = Party_GetCurrentCount(party);
    int count = 0;

    for (i = 0; i < partyCount; i++) {
        if (Pokemon_CanBattle(Party_GetPokemonBySlotIndex(party, i))) {
            count++;
        }
    }

    return count;
}

Pokemon *Party_FindFirstEligibleBattler(const Party *party)
{
    int i;
    int partyCount = Party_GetCurrentCount(party);

    for (i = 0; i < partyCount; i++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(party, i);

        if (Pokemon_CanBattle(mon)) {
            return mon;
        }
    }

    GF_ASSERT(FALSE);
    return NULL;
}

Pokemon *Party_FindFirstHatchedMon(const Party *party)
{
    u16 i;
    u16 partyCount = Party_GetCurrentCount(party);

    for (i = 0; i < partyCount; i++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(party, i);

        if (Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) == FALSE) {
            return mon;
        }
    }

    return NULL;
}

BOOL Party_HasTwoAliveMons(const Party *party)
{
    return Party_AliveMonsCount(party) >= 2;
}

void Party_GiveChampionRibbons(Party *party)
{
    int i;
    u8 championRibbon = TRUE;
    int partyCount = Party_GetCurrentCount(party);

    for (i = 0; i < partyCount; i++) {
        Pokemon *mon = Party_GetPokemonBySlotIndex(party, i);

        if (Pokemon_GetValue(mon, MON_DATA_IS_EGG, NULL) == FALSE) {
            Pokemon_SetValue(mon, MON_DATA_SINNOH_CHAMP_RIBBON, &championRibbon);
        }
    }
}

int Pokemon_DoPoisonDamage(Party *party, u16 param1)
{
    int numPoisoned = 0;
    int numFainted = 0;
    int i, partyCount;
    Pokemon *mon;

    partyCount = Party_GetCurrentCount(party);

    for (i = 0; i < partyCount; i++) {
        mon = Party_GetPokemonBySlotIndex(party, i);

        if (Pokemon_CanBattle(mon)) {
            if (Pokemon_GetValue(mon, MON_DATA_STATUS_CONDITION, NULL) & (MON_CONDITION_TOXIC | MON_CONDITION_POISON)) {
                u32 hp = Pokemon_GetValue(mon, MON_DATA_CURRENT_HP, NULL);

                if (hp > 1) {
                    hp--;
                }

                Pokemon_SetValue(mon, MON_DATA_CURRENT_HP, &hp);

                if (hp == 1) {
                    numFainted++;
                    Pokemon_UpdateFriendship(mon, 7, param1);
                }

                numPoisoned++;
            }
        }
    }

    if (numFainted) {
        return 2;
    } else if (numPoisoned) {
        return 1;
    } else {
        return 0;
    }
}

BOOL Pokemon_TrySurvivePoison(Pokemon *mon)
{
    if (Pokemon_GetValue(mon, MON_DATA_STATUS_CONDITION, NULL) & (MON_CONDITION_TOXIC | MON_CONDITION_POISON)
        && Pokemon_GetValue(mon, MON_DATA_CURRENT_HP, NULL) == 1) {
        u32 condition = MON_CONDITION_NONE;

        Pokemon_SetValue(mon, MON_DATA_STATUS_CONDITION, &condition);
        return TRUE;
    }

    return FALSE;
}
