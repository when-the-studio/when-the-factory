#include "items.h"

ItemSpec g_item_spec_table[] = {
	[ITEM_WOOD_LOG] = {
		.name = "wood log",
		.mass = 10,
		.rect_in_spritesheet = {0, 24, 7, 7},
	},
	[ITEM_TINY_ROCK] = {
		.name = "tiny rock",
		.mass = 4,
		.rect_in_spritesheet = {8, 24, 8, 5},
	},
};
