#define OLC_PGE_APPLICATION

#include "CMemoryGame.h"


int main()
{
	CMemoryGame game;
	if (game.Construct(1024, 768, 1, 1))
	{
		game.Start();
	}
	return 0;
}