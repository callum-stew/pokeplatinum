/*
 * datagen-species
 *
 * Usage: datagen-species <OUT_DIR> <ROOT_DIR> <FORMS_REGISTRY> <TUTOR_SCHEMA>
 *
 * This program is responsible for generating data archive from species data files
 * (res/pokemon/<species>/data.json). Individual files to be polled for packing are
 * drawn from an environment var SPECIES, which should be a semicolon-delimited list
 * of subdirectories of res/pokemon.
 *
 * <FORMS_REGISTRY> is expected to be a listing of additional subdirectories
 * belonging to individual species which have distinct data files. These special
 * forms have their own base stats, types, level-up learnsets, etc., as any base
 * species form would.
 *
 * <TUTOR_SCHEMA> is expected to be a JSON file defining the listing of moves that
 * can be taught by a given move tutor, agnostic of species. This file is only
 * consulted to restrict the set of valid moves in a species' tutorable learnset.
 *
 * The following files are generated by this program:
 *   - pl_personal.narc
 *   - evo.narc
 *   - wotbl.narc
 *   - ppark.narc
 *   - height.narc
 *   - pl_poke_data.narc
 *   - tutorable_moves.h
 *   - species_learnsets_by_tutor.h
 */
#include <algorithm>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <optional>
#include <string>

#include "datagen.h"

#define POKEPLATINUM_GENERATED_ENUM
#define POKEPLATINUM_GENERATED_LOOKUP
#define POKEPLATINUM_GENERATED_LOOKUP_IMPL

#include "generated/abilities.h"
#include "generated/egg_groups.h"
#include "generated/evolution_methods.h"
#include "generated/exp_rates.h"
#include "generated/gender_ratios.h"
#include "generated/items.h"
#include "generated/moves.h"
#include "generated/pal_park_land_area.h"
#include "generated/pal_park_water_area.h"
#include "generated/pokemon_colors.h"
#include "generated/pokemon_types.h"
#include "generated/shadow_sizes.h"
#include "generated/species.h"

#include "struct_defs/archived_poke_sprite_data.h"
#include "struct_defs/species.h"

#define NUM_TMS 92 // TODO: Move this to a more accessible location, maybe?

// This struct serves only one purpose: to ensure that the size of the data written
// to file-iamges in evo.narc is word-aligned. The vanilla structures pad to word-
// alignment with 0s, making for two extra 0-bytes after the array. Without this
// manual alignment, the NARC packing routine would instead pad the virtual files
// with `FF FF`, which obviously breaks matching.
struct SpeciesEvolutionList {
    ALIGN_4 SpeciesEvolution entries[MAX_EVOLUTIONS];
};

// Entries in `wotbl.narc` are dynamically-sized with a terminating sentinel value
// for each entry. So, we need to know how large the learnset itself is for a
// proper malloc and memcpy while packing to the VFS.
struct SpeciesLearnsetWithSize {
    SpeciesLearnset learnset;
    unsigned long size;
};

static const std::string sHeaderMessage = ""
                                          "/*\n"
                                          " * This header was generated by datagen-species; DO NOT MODIFY IT!!!\n"
                                          " */"
                                          "";

static void Usage(std::ostream &ostr)
{
    ostr << "Usage: datagen-species OUT_DIR ROOT_DIR FORMS_REGISTRY TUTOR_SCHEMA" << std::endl;
    ostr << std::endl;
    ostr << "Generates data archives from species data files (res/pokemon/<species>/data.json)" << std::endl;
    ostr << "Species data files to be polled for packing are drawn from the environment var\n"
         << "SPECIES, which must be a semicolon-delimited list of subdirectories of res/pokemon\n"
         << "to be crawled at execution." << std::endl;
}

static SpeciesData ParseSpeciesData(rapidjson::Document &root)
{
    SpeciesData species = { 0 };

    rapidjson::Value &baseStats = root["base_stats"];
    species.baseStats.hp = baseStats["hp"].GetUint();
    species.baseStats.attack = baseStats["attack"].GetUint();
    species.baseStats.defense = baseStats["defense"].GetUint();
    species.baseStats.speed = baseStats["speed"].GetUint();
    species.baseStats.spAttack = baseStats["special_attack"].GetUint();
    species.baseStats.spDefense = baseStats["special_defense"].GetUint();

    rapidjson::Value &evYields = root["ev_yields"];
    species.evYields.hp = evYields["hp"].GetUint();
    species.evYields.attack = evYields["attack"].GetUint();
    species.evYields.defense = evYields["defense"].GetUint();
    species.evYields.speed = evYields["speed"].GetUint();
    species.evYields.spAttack = evYields["special_attack"].GetUint();
    species.evYields.spDefense = evYields["special_defense"].GetUint();

    rapidjson::Value &abilities = root["abilities"];
    species.abilities[0] = LookupConst(abilities[0].GetString(), Ability);
    species.abilities[1] = LookupConst(abilities[1].GetString(), Ability);

    rapidjson::Value &types = root["types"];
    species.types[0] = LookupConst(types[0].GetString(), PokemonType);
    species.types[1] = LookupConst(types[1].GetString(), PokemonType);

    rapidjson::Value &heldItems = root["held_items"];
    species.wildHeldItems.common = LookupConst(heldItems["common"].GetString(), Item);
    species.wildHeldItems.rare = LookupConst(heldItems["rare"].GetString(), Item);

    rapidjson::Value &eggGroups = root["egg_groups"];
    species.eggGroups[0] = LookupConst(eggGroups[0].GetString(), EggGroup);
    species.eggGroups[1] = LookupConst(eggGroups[1].GetString(), EggGroup);

    species.baseExpReward = root["base_exp_reward"].GetUint();
    species.baseFriendship = root["base_friendship"].GetUint();
    species.bodyColor = LookupConst(root["body_color"].GetString(), PokemonColor);
    species.catchRate = root["catch_rate"].GetUint();
    species.expRate = LookupConst(root["exp_rate"].GetString(), ExpRate);
    species.flipSprite = root["flip_sprite"].GetBool();
    species.genderRatio = LookupConst(root["gender_ratio"].GetString(), GenderRatio);
    species.hatchCycles = root["hatch_cycles"].GetUint();
    species.safariFleeRate = root["safari_flee_rate"].GetUint();

    rapidjson::Value &tmLearnset = root["learnset"]["by_tm"];
    for (auto &tmEntry : tmLearnset.GetArray()) {
        std::string entry = tmEntry.GetString();
        int id;
        if (entry[0] == 'T' && entry[1] == 'M') {
            id = std::stoi(entry.substr(2)) - 1;
        } else if (entry[0] == 'H' && entry[1] == 'M') {
            id = std::stoi(entry.substr(2)) - 1 + NUM_TMS;
        } else {
            throw std::invalid_argument("unrecognized TM learnset entry " + entry);
        }
        species.tmLearnsetMasks[id / 32] |= (1 << (id % 32));
    }

    return species;
}

static SpeciesEvolutionList ParseEvolutions(rapidjson::Document &root)
{
    SpeciesEvolutionList evos = { 0 };
    if (!root.HasMember("evolutions")) {
        return evos;
    }

    rapidjson::Value &evoList = root["evolutions"];
    int i = 0;
    for (auto &evoEntry : evoList.GetArray()) {
        EvolutionMethod method = static_cast<EvolutionMethod>(LookupConst(evoEntry[0].GetString(), EvolutionMethod));

        u16 param;
        int speciesIdx = 2;
        switch (method) {
        case EVO_NONE:
        case EVO_LEVEL_HAPPINESS:
        case EVO_LEVEL_HAPPINESS_DAY:
        case EVO_LEVEL_HAPPINESS_NIGHT:
        case EVO_TRADE:
        case EVO_LEVEL_MAGNETIC_FIELD:
        case EVO_LEVEL_MOSS_ROCK:
        case EVO_LEVEL_ICE_ROCK:
            param = 0;
            speciesIdx = 1;
            break;

        case EVO_LEVEL:
        case EVO_LEVEL_ATK_GT_DEF:
        case EVO_LEVEL_ATK_EQ_DEF:
        case EVO_LEVEL_ATK_LT_DEF:
        case EVO_LEVEL_PID_LOW:
        case EVO_LEVEL_PID_HIGH:
        case EVO_LEVEL_NINJASK:
        case EVO_LEVEL_SHEDINJA:
        case EVO_LEVEL_MALE:
        case EVO_LEVEL_FEMALE:
        case EVO_LEVEL_BEAUTY:
            param = evoEntry[1].GetUint();
            break;

        case EVO_TRADE_WITH_HELD_ITEM:
        case EVO_USE_ITEM:
        case EVO_USE_ITEM_MALE:
        case EVO_USE_ITEM_FEMALE:
        case EVO_LEVEL_WITH_HELD_ITEM_DAY:
        case EVO_LEVEL_WITH_HELD_ITEM_NIGHT:
            param = LookupConst(evoEntry[1].GetString(), Item);
            break;

        case EVO_LEVEL_KNOW_MOVE:
            param = LookupConst(evoEntry[1].GetString(), Move);
            break;

        case EVO_LEVEL_SPECIES_IN_PARTY:
            param = LookupConst(evoEntry[1].GetString(), Species);
            break;
        }

        u16 target = LookupConst(evoEntry[speciesIdx].GetString(), Species);
        evos.entries[i++] = SpeciesEvolution {
            .method = static_cast<u16>(method),
            .param = param,
            .targetSpecies = target,
        };
    }

    return evos;
}

static SpeciesLearnsetWithSize ParseLevelUpLearnset(rapidjson::Document &root)
{
    SpeciesLearnsetWithSize result = {};

    rapidjson::Value &byLevel = root["learnset"]["by_level"];
    int i = 0;
    for (auto &byLevelEntry : byLevel.GetArray()) {
        u16 level = byLevelEntry[0].GetUint();
        u16 move = LookupConst(byLevelEntry[1].GetString(), Move);
        result.learnset.entries[i++] = {
            .move = move,
            .level = level,
        };

        if (i == MAX_LEARNSET_ENTRIES) {
            break;
        }
    }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas" // bitfield-constant-conversion is clang-specific; GCC should ignore it
#pragma GCC diagnostic ignored "-Woverflow"
#pragma GCC diagnostic ignored "-Wbitfield-constant-conversion"
    result.learnset.entries[i++] = {
        .move = static_cast<u16>(-1),
        .level = static_cast<u16>(-1),
    };
#pragma GCC diagnostic pop

    result.size = AlignToWord(i * sizeof(SpeciesLearnsetEntry));
    return result;
}

static std::optional<SpeciesPalPark> TryParsePalPark(rapidjson::Document &root)
{
    if (!root.HasMember("catching_show")) {
        return std::nullopt;
    }

    rapidjson::Value &catchingShow = root["catching_show"];
    SpeciesPalPark palPark;
    palPark.landArea = LookupConst(catchingShow["pal_park_land_area"].GetString(), PalParkLandArea);
    palPark.waterArea = LookupConst(catchingShow["pal_park_water_area"].GetString(), PalParkWaterArea);
    palPark.catchingPoints = catchingShow["catching_points"].GetUint();
    palPark.rarity = catchingShow["rarity"].GetUint();
    palPark.unused.asU16 = catchingShow["unused"].GetUint();

    return palPark;
}

static std::vector<Move> EmitTutorableMoves(fs::path &tutorSchemaFname, fs::path outFname)
{
    std::string tutorSchema = ReadWholeFile(tutorSchemaFname);
    rapidjson::Document doc;
    doc.Parse(tutorSchema.c_str());

    std::ofstream header(outFname, std::ios::out | std::ios::trunc | std::ios::ate);
    header << sHeaderMessage << "\n"
           << "#ifndef POKEPLATINUM_GENERATED_TUTORABLE_MOVES\n"
           << "#define POKEPLATINUM_GENERATED_TUTORABLE_MOVES\n"
           << "\n"
           << "static const TeachableMove sTeachableMoves[] = {"
           << std::endl;

    std::vector<Move> tutorables;
    rapidjson::Value &moves = doc["moves"];
    for (const auto &entry : moves.GetObject()) {
        Move tutorable = static_cast<Move>(LookupConst(entry.name.GetString(), Move));
        if (std::find(tutorables.begin(), tutorables.end(), tutorable) == tutorables.end()) {
            tutorables.push_back(tutorable);
        }

        const rapidjson::Value &moveObj = entry.value;
        header << "    { " << entry.name.GetString() << ", "
               << moveObj["redCost"].GetUint() << ", "
               << moveObj["blueCost"].GetUint() << ", "
               << moveObj["yellowCost"].GetUint() << ", "
               << moveObj["greenCost"].GetUint() << ", "
               << moveObj["location"].GetString() << ", }, "
               << std::endl;
    }

    header << "};\n"
           << "\n"
           << "#endif // POKEPLATINUM_GENERATED_TUTORABLE_MOVES"
           << std::endl;
    header.close();
    return tutorables;
}

static void TryEmitTutorableLearnset(rapidjson::Document &root, std::ofstream &ofs, std::vector<Move> &tutorableMoves, std::size_t tutorableLearnsetSize)
{
    const rapidjson::Value &learnsets = root["learnset"];
    if (!learnsets.HasMember("by_tutor")) {
        return;
    }

    const rapidjson::Value &byTutorLearnset = learnsets["by_tutor"];
    std::vector<u8> tutorableLearnset(tutorableLearnsetSize);
    for (const auto &entry : byTutorLearnset.GetArray()) {
        Move tutorable = static_cast<Move>(LookupConst(entry.GetString(), Move));
        std::vector<Move>::iterator it = std::find(tutorableMoves.begin(), tutorableMoves.end(), tutorable);
        if (it == tutorableMoves.end()) {
            std::stringstream ss;
            ss << "Move " << entry.GetString() << " is not available via move tutors";
            throw std::invalid_argument(ss.str());
        }

        // The mask-index of this move is just the move's index / 8
        // The bit-index within that mask is the move's index % 8
        std::size_t idx = it - tutorableMoves.begin();
        tutorableLearnset[idx / 8] |= (1 << (idx % 8));
    }

    ofs << "    { ";
    for (const auto &mask : tutorableLearnset) {
        ofs << "0x" << std::setfill('0') << std::setw(2) << (int)mask << ", ";
    }
    ofs << "},\n";
}

static void PackHeights(vfs_pack_ctx *vfs, rapidjson::Document &root, u8 genderRatio)
{
    u8 *backFemale, *backMale, *frontFemale, *frontMale;
    const rapidjson::Value &backOffsets = root["back"]["y_offset"];
    const rapidjson::Value &frontOffsets = root["front"]["y_offset"];

    u32 femaleSize = 1, maleSize = 1;

    if (genderRatio == GENDER_RATIO_FEMALE_ONLY) {
        backMale = static_cast<u8 *>(malloc(0));
        frontMale = static_cast<u8 *>(malloc(0));
        maleSize = 0;
    } else {
        backMale = static_cast<u8 *>(malloc(1));
        frontMale = static_cast<u8 *>(malloc(1));
        *backMale = backOffsets["male"].GetUint();
        *frontMale = frontOffsets["male"].GetUint();
    }

    if (genderRatio == GENDER_RATIO_MALE_ONLY || genderRatio == GENDER_RATIO_NO_GENDER) {
        backFemale = static_cast<u8 *>(malloc(0));
        frontFemale = static_cast<u8 *>(malloc(0));
        femaleSize = 0;
    } else {
        backFemale = static_cast<u8 *>(malloc(1));
        frontFemale = static_cast<u8 *>(malloc(1));
        *backFemale = backOffsets["female"].GetUint();
        *frontFemale = frontOffsets["female"].GetUint();
    }

    narc_pack_file(vfs, backFemale, femaleSize);
    narc_pack_file(vfs, backMale, maleSize);
    narc_pack_file(vfs, frontFemale, femaleSize);
    narc_pack_file(vfs, frontMale, maleSize);
}

static SpriteAnimationFrame ParseSpriteAnimationFrame(const rapidjson::Value &frame)
{
    SpriteAnimationFrame data = { 0 };
    data.spriteFrame = frame["sprite_frame"].GetInt();
    data.frameDelay = frame["frame_delay"].GetUint();
    data.xOffset = frame["x_shift"].GetInt();
    data.yOffset = frame["y_shift"].GetInt();

    return data;
}

static PokeSpriteFaceData ParsePokeSpriteFace(const rapidjson::Value &face)
{
    PokeSpriteFaceData data = { 0 };
    data.animation = face["animation"].GetUint();
    data.cryDelay = face["cry_delay"].GetUint();
    data.startDelay = face["start_delay"].GetUint();

    int i = 0;
    for (auto &frame : face["frames"].GetArray()) {
        data.frames[i++] = ParseSpriteAnimationFrame(frame);
    }

    return data;
}

static ArchivedPokeSpriteData ParsePokeSprite(const rapidjson::Document &root)
{
    ArchivedPokeSpriteData data = { 0 };

    const rapidjson::Value &front = root["front"];
    const rapidjson::Value &back = root["back"];
    const rapidjson::Value &shadow = root["shadow"];

    std::cout << "here" << std::endl;

    data.faces[0] = ParsePokeSpriteFace(front);
    data.faces[1] = ParsePokeSpriteFace(back);
    data.yOffset = front["addl_y_offset"].GetInt();
    data.xOffsetShadow = shadow["x_offset"].GetInt();
    data.shadowSize = LookupConst(shadow["size"].GetString(), ShadowSize);

    return data;
}

int main(int argc, char **argv)
{
    if (argc == 1) {
        Usage(std::cout);
        return EXIT_SUCCESS;
    }

    fs::path outputRoot = argv[1];
    fs::path dataRoot = argv[2];
    fs::path formsRegistryFname = argv[3];
    fs::path tutorSchemaFname = argv[4];

    // Determine what moves are tutorable and output the corresponding C header.
    std::vector<Move> tutorableMoves = EmitTutorableMoves(tutorSchemaFname, outputRoot / "tutorable_moves.h");

    // Bootstrap the by-tutor learnsets header.
    std::ofstream byTutorMovesets(outputRoot / "species_learnsets_by_tutor.h");
    byTutorMovesets << sHeaderMessage << "\n"
                    << "#ifndef POKEPLATINUM_GENERATED_SPECIES_LEARNSETS_BY_TUTOR_H\n"
                    << "#define POKEPLATINUM_GENERATED_SPECIES_LEARNSETS_BY_TUTOR_H\n"
                    << "\n"
                    << "#include \"tutor_movesets.h\"\n"
                    << "\n"
                    << "static const MovesetMask sSpeciesLearnsetsByTutor[MOVESET_MAX] = {\n";
    byTutorMovesets << std::hex << std::setiosflags(std::ios::uppercase); // render all numeric inputs to the stream as hexadecimal

    // Tutorable learnsets are stored as an array of bitmasks; each bit in the mask
    // denotes if a tutorable move can be learned by a given species.
    std::size_t tutorableLearnsetSize = (tutorableMoves.size() + 7) / 8;

    // Prepare loop contents.
    std::vector<std::string> speciesRegistry = ReadRegistryEnvVar("SPECIES");
    std::vector<std::string> formsRegistry = ReadFileLines(formsRegistryFname);
    speciesRegistry.insert(speciesRegistry.end(), formsRegistry.begin(), formsRegistry.end());
    std::vector<std::string>::iterator lastNatDex = speciesRegistry.end() - formsRegistry.size() - 3; // -3 accounts for egg and bad_egg

    // Prepare VFSes for each NARC to be output.
    vfs_pack_ctx *personalVFS = narc_pack_start();
    vfs_pack_ctx *evoVFS = narc_pack_start();
    vfs_pack_ctx *wotblVFS = narc_pack_start();
    vfs_pack_ctx *heightVFS = narc_pack_start();
    std::vector<SpeciesPalPark> palParkData;
    std::vector<ArchivedPokeSpriteData> pokeSpriteData;

    rapidjson::Document doc;
    for (auto &species : speciesRegistry) {
        try {
            fs::path speciesDataPath = dataRoot / species / "data.json";
            std::string json = ReadWholeFile(speciesDataPath);
            doc.Parse(json.c_str());

            SpeciesData data = ParseSpeciesData(doc);
            SpeciesEvolutionList evos = ParseEvolutions(doc);
            SpeciesLearnsetWithSize sizedLearnset = ParseLevelUpLearnset(doc);
            std::optional<SpeciesPalPark> palPark = TryParsePalPark(doc);
            TryEmitTutorableLearnset(doc, byTutorMovesets, tutorableMoves, tutorableLearnsetSize);

            narc_pack_file_copy(personalVFS, reinterpret_cast<unsigned char *>(&data), sizeof(data));
            narc_pack_file_copy(evoVFS, reinterpret_cast<unsigned char *>(&evos), sizeof(evos));
            narc_pack_file_copy(wotblVFS, reinterpret_cast<unsigned char *>(&sizedLearnset.learnset), sizedLearnset.size);

            if (palPark.has_value()) {
                palParkData.emplace_back(palPark.value());
            }

            fs::path speciesSpriteDataPath = dataRoot / species / "sprite_data.json";
            std::ifstream spriteDataIFS(speciesSpriteDataPath);
            if (spriteDataIFS.good()) {
                std::string spriteData = ReadWholeFile(spriteDataIFS);
                doc.Parse(spriteData.c_str());

                u8 genderRatio = species != "none" ? data.genderRatio : GENDER_RATIO_FEMALE_50; // treat SPECIES_NONE as if it has two genders.
                PackHeights(heightVFS, doc, genderRatio);

                ArchivedPokeSpriteData pokeSprite = ParsePokeSprite(doc);
                pokeSpriteData.emplace_back(pokeSprite);
            }
        } catch (std::exception &e) {
            std::cerr << "exception parsing data file for " + species << std::endl;
            std::cerr << e.what() << std::endl;
            std::exit(EXIT_FAILURE);
        }
    }

    byTutorMovesets << "};\n"
                    << "#endif // POKEPLATINUM_GENERATED_SPECIES_LEARNSETS_BY_TUTOR_H"
                    << std::endl;
    byTutorMovesets.close();

    PackNarc(personalVFS, outputRoot / "pl_personal.narc");
    PackNarc(evoVFS, outputRoot / "evo.narc");
    PackNarc(wotblVFS, outputRoot / "wotbl.narc");
    PackNarc(heightVFS, outputRoot / "height.narc");
    PackSingleFileNarc(palParkData, outputRoot / "ppark.narc");
    PackSingleFileNarc(pokeSpriteData, outputRoot / "pl_poke_data.narc");
    return EXIT_SUCCESS;
}
