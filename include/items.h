#ifndef WHEN_THE_FACTORY_ITEM_
#define WHEN_THE_FACTORY_ITEM_

#include <SDL2/SDL.h>
#include "utils.h"

struct ItemSpec {
	char* name;
	int mass;
	SDL_Rect rect_in_spritesheet;
};
typedef struct ItemSpec ItemSpec;

enum ItemType {
	ITEM_WOOD_LOG,
	ITEM_TINY_ROCK,
};
typedef enum ItemType ItemType;

extern ItemSpec g_item_spec_table[];

struct Item {
	ItemType type;
};
typedef struct Item Item;

struct ItemStack {
	Item item;
	int count;
};
typedef struct ItemStack ItemStack;

struct Inventory {
	DA(ItemStack) stacks;
};
typedef struct Inventory Inventory;

#endif // WHEN_THE_FACTORY_ITEM_
