#ifndef __UITOOLTIP_H__
#define __UITOOLTIP_H__

#pragma once

namespace DuiLib
{
	class UILIB_API CTooltip
	{
	public:
		CTooltip();
		~CTooltip();
		void Open(HINSTANCE hInstance, HWND hWndPaint, LPCTSTR szText, RECT const& rc);
		void Close();
		DWORD GetDelayTime();

	protected:
		TOOLINFO  m_ToolTip;
		HWND      m_hWndTooltip;
	};
}

#endif
