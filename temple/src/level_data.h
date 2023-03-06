#pragma once

typedef enum {
	LEVEL_NORMAL,
	LEVEL_BONUS,
	LEVEL_BOSS
} LevelType;

typedef struct {
	uint minScore;
	uint maxTime;
} LevelObjectives;

typedef struct {
	const char* assetName;
	const char* shortName;
	const char* displayName;
	LevelType type;
	LevelObjectives objectives;
} LevelInfo;

#define LEVEL_COUNT 11
inline LevelInfo* GetAllLevels() {
	LevelInfo levels[] = {
		{"map_level001", "01", "UNNAMED LEVEL", LEVEL_NORMAL, {10000,180}},
		{"map_level002", "01", "UNNAMED LEVEL", LEVEL_NORMAL, {10000,180}},
		{"map_fluffoverload", "01", "UNNAMED LEVEL", LEVEL_NORMAL, {10000,180}},
		{"map_level003", "01", "UNNAMED LEVEL", LEVEL_NORMAL, {10000,180}},
		{"map_level004", "01", "UNNAMED LEVEL", LEVEL_NORMAL, {10000,180}},
		{"map_level005", "01", "UNNAMED LEVEL", LEVEL_NORMAL, {10000,180}},
		{"map_level006", "01", "UNNAMED LEVEL", LEVEL_NORMAL, {10000,180}},
		{"map_level007", "01", "UNNAMED LEVEL", LEVEL_NORMAL, {10000,180}},
		{"map_level008", "01", "UNNAMED LEVEL", LEVEL_NORMAL, {10000,180}},
		{"map_end_boss", "01", "UNNAMED LEVEL", LEVEL_NORMAL, {10000,180}},

		{"map_bonus_cave", "A", "UNNAMED BONUS LEVEL", LEVEL_BONUS, {10000,180}},
		{"map_bonus_pit", "B", "UNNAMED BONUS LEVEL", LEVEL_BONUS, {10000,180}},
	};
	return levels;
}

#ifdef IMPLEMENT_TEMPLE_LEVEL_DATA

#endif
