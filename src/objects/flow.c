#include "flow.h"

CardinalType getOpposedDirection(CardinalType direction){
	/* We could also do 
		return (CardinalType)((direction+2)%4)
		(performance ?)
	 */
	switch (direction){
		case NORTH: return SOUTH;
		case SOUTH: return NORTH;
		case WEST:	return EAST;
		case EAST:	return WEST;
		default:		return NORTH;
	}
}
