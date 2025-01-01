#ifndef POKEPLATINUM_CATCHING_SHOW_H
#define POKEPLATINUM_CATCHING_SHOW_H

#include "field/field_system_decl.h"

#include "field_battle_data_transfer.h"

#define POINTS_LOST_PER_SECOND 2
#define CATCHING_SHOW_MONS     6
#define WEIGHT_NO_ENCOUNTER    20
#define DISTINCT_TYPE_BONUS    50
#define DIFFERENT_TYPE_BONUS   200
#define MAX_TIME_SECONDS       1000

enum PAL_PARK_ENCOUNTER_TYPE {
    PAL_PARK_ENCOUNTER_TYPE_NONE = 0,
    PAL_PARK_ENCOUNTER_LAND_ZONE_1 = 1,
    PAL_PARK_ENCOUNTER_WATER_ZONE_1 = 5
};

typedef struct CatchingShowPokemon {
    u16 species;
    u8 encounterType;
    u8 rarity;
    u16 catchingPoints;
    u8 type1;
    u8 type2;
} CatchingShowPokemon;

typedef struct CatchingShow {
    CatchingShowPokemon pokemon[CATCHING_SHOW_MONS];
    u8 caughtMonsOrder[CATCHING_SHOW_MONS];
    int steps;
    int currentEncounterIndex;
    s64 startTime;
    int timePoints;
} CatchingShow;

void CatchingShow_Start(FieldSystem *fieldSystem);
void CatchingShow_End(FieldSystem *fieldSystem);
BOOL CatchingShow_CheckWildEncounter(FieldSystem *fieldSystem, int playerX, int playerY);
FieldBattleDTO *CatchingShow_GetBattleDTO(FieldSystem *fieldSystem);
void CatchingShow_UpdateBattleResult(FieldSystem *fieldSystem, FieldBattleDTO *dto);
int CatchingShow_GetParkBallCount(FieldSystem *fieldSystem);
int CatchingShow_CalcCatchingPoints(FieldSystem *fieldSystem);
int CatchingShow_GetTimePoints(FieldSystem *fieldSystem);
int CatchingShow_GetTypePoints(FieldSystem *fieldSystem);

#endif // POKEPLATINUM_CATCHING_SHOW_H
