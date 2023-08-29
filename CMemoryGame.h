#pragma once

#include <list>
#include <vector>
#include "olcPixelGameEngine.h"

typedef struct {
	int nCard;
	int nUsed;
	olc::Pixel px;
} SCARD;

typedef struct {
	int ixCard;
	bool bSelected;
	bool bShow;
	olc::vf2d vPos;
} SMATRIX;

typedef struct {
	int nElapsedSeconds;
	long nScore;
} SSCORE;

typedef struct {
	std::string sIsoDate;
	int nScore;
} SHIGHSCORE;


class CMemoryGame : public olc::PixelGameEngine
{

private:
	int _nState;
	std::vector<SCARD> _vecCards;
	std::vector<SMATRIX> _vecDeskMatrix;
	olc::vf2d _vBallStartPos;
	int _cnFailedAttempts;
	int _cnSelected;
	int _cnRemoved;
	float _fFullSelectedElapsed;
	float _fStateTime;
	SSCORE _score;
	std::vector<SHIGHSCORE> _vecHighScores;

public:
	CMemoryGame();

	bool OnUserCreate() override;
	bool OnUserUpdate(float fElapsedTime) override;

private:
	void DrawButton(olc::vf2d vPos, olc::vf2d vSize, std::string sText, olc::Pixel clrBtn, olc::Pixel clrTxt);

	void DoSplashScreen(float fElapsedTime);
	void DoGame(float fElapsedTime);
	void DoGameEnd(float fElapsedTime);
	void DoHighScoreInit(float fElapsedTime);
	void DoHighScore(float fElapsedTime);

	void ResetGame();
	void InitializeCards();
	void InitializeMatrix();

	bool ReadHighScores(std::vector<SHIGHSCORE>& vecHighScores);
	bool WriteHighScores(std::vector<SHIGHSCORE>& vecHighScores);
};

