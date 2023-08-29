#include "CButton.h"

CButton::CButton()
{
	
}

void CButton::RecalcButton(const std::string& sText, olc::vf2d &vPos, olc::vf2d &vSize)
{
	_sText = sText;
	_vPos = vPos;
	_vSize = vSize;
	_vScale = { 2.5f, 2.5f };
	float fSizeText = _sText.size() * 8.0f * _vScale.x;
	float fPaddingX = (_vSize.x - fSizeText) / 2.0f;
	_vPadding = { fPaddingX, 20.0f };
}

void CButton::DrawSelf(olc::PixelGameEngine* pge)
{
	
	pge->FillRectDecal(_vPos, _vSize, olc::DARK_GREEN);
	pge->DrawStringDecal(_vPos + _vPadding, _sText, olc::WHITE, _vScale);
	if (IsMouseOver(pge))
	{
		pge->DrawRectDecal(_vPos, _vSize, olc::GREEN);
	}
}

bool CButton::IsClicked(olc::PixelGameEngine* pge)
{
	if (!pge->IsFocused())
	{
		return false;
	}
	return pge->GetMouse(0).bReleased && IsMouseOver(pge);
}

bool CButton::IsMouseOver(olc::PixelGameEngine* pge)
{
	if (!pge->IsFocused())
	{
		return false;
	}
	olc::vi2d vPosMouse = pge->GetMousePos();
	if (vPosMouse.x > _vPos.x && vPosMouse.x < (_vPos.x + _vSize.x)
		&& vPosMouse.y > _vPos.y && vPosMouse.y < (_vPos.y + _vSize.y))
	{
		return true;
	}
	return false;
}