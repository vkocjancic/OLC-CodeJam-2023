#pragma once

#include "olcPixelGameEngine.h"

class CButton
{

private:
	std::string _sText;
	olc::vf2d _vPos;
	olc::vf2d _vSize;
	olc::vf2d _vPadding;
	olc::vf2d _vScale;

public:
	CButton();

	void RecalcButton(const std::string& sText, olc::vf2d &vPos, olc::vf2d &vSize);
	void DrawSelf(olc::PixelGameEngine* pge);
	bool IsClicked(olc::PixelGameEngine* pge);

private:
	bool IsMouseOver(olc::PixelGameEngine* pge);

};

