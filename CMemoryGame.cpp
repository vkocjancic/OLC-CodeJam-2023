#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <chrono>
#include <format>

#include "mgconst.h"
#include "CMemoryGame.h"

CMemoryGame::CMemoryGame() 
{
	sAppName = "Memory - the game";
	_nState = STATE_SPLASH;
}

bool CMemoryGame::OnUserCreate()
{	
	_fStateTime = 0.0f;
	_sprLogo = std::make_unique<olc::Sprite>("./graphics/logo.png");

	return true;
}

bool CMemoryGame::OnUserUpdate(float fElapsedTime)
{
	if (_nState == STATE_SPLASH)
	{
		DoSplashScreen(fElapsedTime);
	}
	else if (_nState == STATE_GAME_INIT)
	{
		ResetGame();
		InitializeCards();
		InitializeMatrix();
		_nState = STATE_GAME;
	}
	else if (_nState == STATE_GAME)
	{
		DoGame(fElapsedTime);
	}
	else if (_nState == STATE_GAME_END)
	{
		DoGameEnd(fElapsedTime);
	}
	else if (_nState == STATE_HIGHSCORE_INIT)
	{
		DoHighScoreInit(fElapsedTime);
	}
	else if (_nState == STATE_HIGHSCORE)
	{
		DoHighScore(fElapsedTime);
	}
	return true;
}

void CMemoryGame::DrawButton(olc::vf2d vPos, olc::vf2d vSize, std::string sText, olc::Pixel clrBtn, olc::Pixel clrTxt)
{
	FillRectDecal(vPos, vSize, clrBtn);
	DrawStringDecal({ vPos.x + 20.0f, vPos.y + 20.0f }, sText, clrTxt, { 2.5f, 2.5f });
}

void CMemoryGame::DoSplashScreen(float fElapsedTime)
{
	_fStateTime += fElapsedTime;

	olc::vf2d vPosBtnNewGame = { ScreenWidth() / 2.0f - 130.0f, ScreenHeight() / 2.0f };
	olc::vf2d vSizeBtn = { 250.0f, 60.0f };

	_btnNewGame.RecalcButton("New game", vPosBtnNewGame, vSizeBtn);
	if (_btnNewGame.IsClicked(this))
	{
		_nState = STATE_GAME_INIT;
		_fStateTime = 0.0f;
	}

	olc::vf2d vPosBtnHighScores = { vPosBtnNewGame.x, vPosBtnNewGame.y + vSizeBtn.y + 15.0f };
	_btnHighScores.RecalcButton("High scores", vPosBtnHighScores, vSizeBtn);
	if (_btnHighScores.IsClicked(this))
	{
		_nState = STATE_HIGHSCORE_INIT;
		_fStateTime = 0.0f;
	}

	// draw on screen
	Clear(olc::BLACK);
	
	// draw logo
	SetPixelMode(olc::Pixel::MASK); // Dont draw pixels which have any transparency
	DrawSprite({ ScreenWidth() / 2 - 400, ScreenHeight() / 2 - 300 }, _sprLogo.get());
	SetPixelMode(olc::Pixel::NORMAL); // Draw all pixels

	_btnNewGame.DrawSelf(this);
	_btnHighScores.DrawSelf(this);
}

void CMemoryGame::DoGame(float fElapsedTime)
{
	_fStateTime += fElapsedTime;
	int nMatrixSize = _vecDeskMatrix.size();

	// recalc

	// if 3 are selected, count down clock for animation purposes
	if (_fFullSelectedElapsed > 0.0f && _cnSelected == 3)
	{
		_fFullSelectedElapsed -= fElapsedTime;
	}

	// if 3 are selected and clock ran out:
	// - check if all three selected are equal
	// - if eqal, add to score
	// - deselect all
	if (_fFullSelectedElapsed <= 0.0f && _cnSelected == 3)
	{
		// check if all three selected are equal
		bool bSelectedEqual = true;
		int nLastSelectedCard = -1;
		for (int i = 0; i < nMatrixSize; i++)
		{
			SMATRIX* pMtx = &_vecDeskMatrix[i];
			if (!pMtx->bSelected)
			{
				continue;
			}
			if (nLastSelectedCard != -1)
			{
				bSelectedEqual &= nLastSelectedCard == pMtx->ixCard;
			}
			if (!bSelectedEqual)
			{
				break;
			}
			nLastSelectedCard = pMtx->ixCard;
		}

		if (bSelectedEqual)
		{
			// luck guesses get rewarded. But only until 3 triples are left
			if (_cnFailedAttempts == 0 && _cnRemoved < (nMatrixSize - 9))
			{
				_score.nScore += 10 * REWARD;
			}
			else
			{
				// because you can have lucky guess in last 3 triples...
				if (_cnFailedAttempts == 0)
				{
					_score.nScore += REWARD;
				}
				// for everything else, adjust score
				else
				{
					_score.nScore += REWARD / _cnFailedAttempts;
				}
				// minimal reward is 1
				if (_score.nScore == 0) 
				{
					_score.nScore = 1;
				}
			}
			_cnFailedAttempts = 0;
			_cnRemoved += _cnSelected;

			// if all balls were removed, end game
			if (_cnRemoved == nMatrixSize)
			{
				_score.nElapsedSeconds = (int)_fStateTime;
				_fStateTime = 0.0f;
				_nState = STATE_GAME_END;
			}
		}
		else
		{
			_cnFailedAttempts++;
		}

		// deselect all
		_fFullSelectedElapsed = -1.0f;
		_cnSelected = 0;
		for (int i = 0; i < nMatrixSize; i++)
		{
			SMATRIX* pMtx = &_vecDeskMatrix[i];
			if (pMtx->bSelected && bSelectedEqual)
			{
				pMtx->bShow = false;
			}
			pMtx->bSelected = false;
		}
	}

	// if 3 are not selected and left-mouse button is pressed:
	// - check if click occured on card
	// - and card is not already selected
	if (_cnSelected < 3 && GetMouse(0).bReleased)
	{
		olc::vi2d vMousePos = GetMousePos();
		for (int i = 0; i < nMatrixSize; i++)
		{
			SMATRIX* pMtx = &_vecDeskMatrix[i];
			if (vMousePos.x > pMtx->vPos.x - RADIUS && vMousePos.x < pMtx->vPos.x + RADIUS
				&& vMousePos.y > pMtx->vPos.y - RADIUS && vMousePos.y < pMtx->vPos.y + RADIUS
				&& !pMtx->bSelected)
			{
				pMtx->bSelected = true;
				_cnSelected++;
			}
		}
	}

	// check if card must be focused or not
	if (IsFocused())
	{
		olc::vi2d vMousePos = GetMousePos();
		for (int i = 0; i < nMatrixSize; i++)
		{
			SMATRIX* pMtx = &_vecDeskMatrix[i];
			pMtx->bFocused = false;
			if (vMousePos.x > pMtx->vPos.x - RADIUS && vMousePos.x < pMtx->vPos.x + RADIUS
				&& vMousePos.y > pMtx->vPos.y - RADIUS && vMousePos.y < pMtx->vPos.y + RADIUS)
			{
				pMtx->bFocused = true;
			}
		}
	}

	// if 3 are selected, set animation timeout
	if (_cnSelected == 3 && _fFullSelectedElapsed == -1.0f)
	{
		_fFullSelectedElapsed = 0.5f;
	}

	// calc score output
	std::string sScore;
	sScore.resize(18);
	sScore = "Score: " + std::to_string(_score.nScore);

	// calc time output
	std::string sTime;
	sTime.resize(30);
	int nSecondsElapsed = (int)_fStateTime % 60;
	int nMinutesElapsed = (int)_fStateTime / 60;
	std::string sSeconds = std::to_string(nSecondsElapsed);
	if (nSecondsElapsed < 10)
	{
		sSeconds = "0" + sSeconds;
	}
	std::string sMinutes = std::to_string(nMinutesElapsed);
	if (nMinutesElapsed < 10)
	{
		sMinutes = "0" + sMinutes;
	}
	sTime = "Time elapsed: " + sMinutes + ":" + sSeconds;

	// render
	Clear(olc::BLACK);

	DrawStringDecal({ ScreenWidth() - 200.0f, 10.0f }, sScore, olc::DARK_GREEN, {1.5f, 1.5f});
	DrawStringDecal({ 150.0f, 10.0f }, sTime, olc::GREY, { 1.5f, 1.5f });

	// draw cards
	olc::vf2d vBall = { _vBallStartPos.x, _vBallStartPos.x };
	for (int i = 0; i < nMatrixSize; i++)
	{
		SMATRIX mtx = _vecDeskMatrix[i];
		if (!mtx.bShow)
		{
			continue;
		}
		olc::Pixel pxBall = olc::DARK_GREY;
		SCARD card = _vecCards[mtx.ixCard];
		if (mtx.bSelected)
		{
			pxBall = card.px;
			FillCircle(mtx.vPos, RADIUS, pxBall);
		}
		else if (mtx.bFocused)
		{
			pxBall = olc::DARK_YELLOW;
			DrawCircle(mtx.vPos, RADIUS, pxBall);
		}
		else 
		{
			DrawCircle(mtx.vPos, RADIUS, pxBall);
		}
	}
}

void CMemoryGame::DoGameEnd(float fElapsedTime)
{
	// recalc
	_fStateTime += fElapsedTime;
	if (_fStateTime >= 3.0f)
	{
		_fStateTime = 0.0f;
		_nState = STATE_HIGHSCORE_INIT;
	}

	int nTotal = _score.nScore + (-10) * _score.nElapsedSeconds;
	if (nTotal < 0)
	{ 
		nTotal = 0;
	}
	std::string sScore = "Score:   " + std::to_string(_score.nScore);
	std::string sTime = "Seconds: " + std::to_string(_score.nElapsedSeconds) + " (-10 points per second)";
	std::string sTotal = "Total:   " + std::to_string(nTotal);

	// render
	Clear(olc::BLACK);

	DrawStringDecal({ ScreenWidth() / 2 - 200.0f, 200.0f }, "You did it!", olc::DARK_GREEN, { 3.0f, 3.0f });
	DrawStringDecal({ ScreenWidth() / 2 - 200.0f, 300.0f }, sScore, olc::WHITE, { 1.5f, 1.5f });
	DrawStringDecal({ ScreenWidth() / 2 - 200.0f, 320.0f }, sTime, olc::WHITE, { 1.5f, 1.5f });
	DrawStringDecal({ ScreenWidth() / 2 - 200.0f, 340.0f }, "----------------------------------------", olc::WHITE, { 1.5f, 1.5f });
	DrawStringDecal({ ScreenWidth() / 2 - 200.0f, 360.0f }, sTotal, olc::MAGENTA, { 1.5f, 1.5f });
}

void CMemoryGame::DoHighScoreInit(float fElapsedTime)
{
	_fStateTime += fElapsedTime;

	// read hich scores from file
	ReadHighScores(_vecHighScores);

	// get size of array
	size_t nSize = _vecHighScores.size();

	// add current score
	auto now = std::chrono::floor<std::chrono::days>(std::chrono::system_clock::now());
	SHIGHSCORE highScore;
	highScore.nScore = _score.nScore;
	highScore.sIsoDate = std::format("{0:%Y-%m-%d}",std::chrono::year_month_day{ now });
	_vecHighScores.push_back(highScore);

	// sort array by score
	std::sort(_vecHighScores.begin(), _vecHighScores.end(), [this](SHIGHSCORE a, SHIGHSCORE b) {
		return a.nScore > b.nScore;
	});

	// resize array back to original size
	_vecHighScores.resize(nSize);
	
	// write high scores back
	WriteHighScores(_vecHighScores);
	_nState = STATE_HIGHSCORE;
}

void CMemoryGame::DoHighScore(float fElapsedTime)
{
	// recalc
	_fStateTime += fElapsedTime;

	olc::vf2d vPosBtnMainMenu = { ScreenWidth() / 2.0f - 100.0f, ScreenHeight() / 2.0f + 200.0f };
	olc::vf2d vSizeBtn = { 250.0f, 60.0f };

	_btnMainMenu.RecalcButton("Main menu", vPosBtnMainMenu, vSizeBtn);
	if (_btnMainMenu.IsClicked(this))
	{
		_nState = STATE_SPLASH;
		_fStateTime = 0.0f;
	}

	// render
	Clear(olc::BLACK);

	olc::vf2d vPos = { ScreenWidth() / 2 - 200.0f, 100.0f };
	DrawStringDecal(vPos, "Highest of scores", olc::DARK_GREEN, { 3.0f, 3.0f });
	vPos.y += 100.0f;
	for (auto score : _vecHighScores)
	{
		if (score.nScore < 0)
		{
			break;
		}
		std::string sHighScore = std::format("{}      {:18}", score.sIsoDate, score.nScore);
		DrawStringDecal(vPos, sHighScore, olc::WHITE, { 1.5f, 1.5f });
		vPos.y += 30;
	}

	_btnMainMenu.DrawSelf(this);
}


void CMemoryGame::ResetGame()
{
	_vBallStartPos = { ScreenWidth() / 2.0f - 350.0f , ScreenHeight() / 2.0f - 300.0f };
	_cnFailedAttempts = 0;
	_cnSelected = 0;
	_cnRemoved = 0;
	_score.nElapsedSeconds = 0;
	_score.nScore = 0;
	_fFullSelectedElapsed = -1.0f;

	srand(std::time(0));
}

void CMemoryGame::InitializeCards()
{
	_vecCards.clear();
	for (int i = 0; i < MAX_CARDS; i++)
	{
		SCARD card;
		card.nCard = i;
		card.nUsed = 0;
		switch (card.nCard)
		{
		case 0:
			card.px = olc::VERY_DARK_BLUE;
			break;
		case 1:
			card.px = olc::RED;
			break;
		case 2:
			card.px = olc::YELLOW;
			break;
		case 3:
			card.px = olc::GREEN;
			break;
		case 4:
			card.px = olc::CYAN;
			break;
		case 5:
			card.px = olc::BLUE;
			break;
		case 6:
			card.px = olc::MAGENTA;
			break;
		case 7:
			card.px = olc::VERY_DARK_MAGENTA;
			break;
		case 8:
			card.px = olc::DARK_GREY;
			break;
		case 9:
			card.px = olc::DARK_RED;
			break;
		}
		_vecCards.push_back(card);
	}
}

void CMemoryGame::InitializeMatrix()
{
	_vecDeskMatrix.clear();
	// initialize game matrix
	for (int i = 0; i < MATRIX_ROWS * MATRIX_COLS; i++)
	{
		bool bIsOk = false;
		SMATRIX mtx;
		mtx.vPos.x = _vBallStartPos.x + ((i % MATRIX_COLS) + 1) * 100.0f;
		mtx.vPos.y = _vBallStartPos.y + ((i / MATRIX_COLS) + 1) * 100.0f;
		mtx.bFocused = false;
		mtx.bSelected = false;
		mtx.bShow = true;

		do
		{
			int ixCard = rand() % MAX_CARDS;
			if (_vecCards[ixCard].nUsed < 3)
			{
				_vecCards[ixCard].nUsed++;
				mtx.ixCard = ixCard;
				_vecDeskMatrix.push_back(mtx);
				bIsOk = true;
			}
		} while (!bIsOk);
	}
}

bool CMemoryGame::ReadHighScores(std::vector<SHIGHSCORE>& vecHighScores)
{
	vecHighScores.resize(MAX_HIGH_SCORES);
	for (int i = 0; i < vecHighScores.size(); i++) 
	{
		vecHighScores[i].nScore = -1;
	}

	FILE *fp;
	fopen_s(&fp, "hs.dat", "r");
	if (NULL == fp)
	{
		return false;
	}
	
	char pszBuf[30] = {};
	int ix = 0; 
	while (fgets(pszBuf, 30, fp) != NULL)
	{
		if (ix >= vecHighScores.size())
		{
			break;
		}
		SHIGHSCORE *pScore = &vecHighScores[ix];
		char pszBufIn[11] = {};
		int nScore = 0;
		sscanf_s(pszBuf, "%s\t%d\n", pszBufIn, (unsigned)_countof(pszBufIn), &nScore);
		pScore->nScore = nScore;
		pScore->sIsoDate.assign(pszBufIn, 10);
		ix++;
	}

	fclose(fp);

	return true;
}

bool CMemoryGame::WriteHighScores(std::vector<SHIGHSCORE>& vecHighScores)
{
	FILE* fp;
	fopen_s(&fp, "hs.dat", "w+");
	if (NULL == fp)
	{
		return false;
	}

	for (int i = 0; i < vecHighScores.size(); i++)
	{
		if (vecHighScores[i].nScore < 0)
		{
			break;
		}
		fprintf(fp, "%s\t%d\n", vecHighScores[i].sIsoDate.c_str(), vecHighScores[i].nScore);
	}

	fclose(fp);

	return true;
}