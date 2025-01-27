#include "pokedex_data_index.h"

#include <nitro.h>
#include <string.h>

#include "generated/text_banks.h"

#include "narc.h"

static u32 sPokedexDataNarcIndex = NARC_INDEX_APPLICATION__ZUKANLIST__ZKN_DATA__ZUKAN_DATA;
static u32 sWeightMessageBankIndex = TEXT_BANK_SPECIES_WEIGHT;
static u32 sHeightMessageBankIndex = TEXT_BANK_SPECIES_HEIGHT;

void Pokedex_SetupGiratina(u32 formNun)
{
    if (formNun == 0) {
        sPokedexDataNarcIndex = NARC_INDEX_APPLICATION__ZUKANLIST__ZKN_DATA__ZUKAN_DATA_GIRA;
        sWeightMessageBankIndex = TEXT_BANK_SPECIES_WEIGHT_GIRA;
        sHeightMessageBankIndex = TEXT_BANK_SPECIES_HEIGHT_GIRA;
    } else {
        sPokedexDataNarcIndex = NARC_INDEX_APPLICATION__ZUKANLIST__ZKN_DATA__ZUKAN_DATA;
        sWeightMessageBankIndex = TEXT_BANK_SPECIES_WEIGHT;
        sHeightMessageBankIndex = TEXT_BANK_SPECIES_HEIGHT;
    }
}

u32 Pokedex_Data_NARC_Index(void)
{
    return sPokedexDataNarcIndex;
}

u32 Weight_Message_Bank_Index(void)
{
    return sWeightMessageBankIndex;
}

u32 Height_Message_Bank_Index(void)
{
    return sHeightMessageBankIndex;
}
