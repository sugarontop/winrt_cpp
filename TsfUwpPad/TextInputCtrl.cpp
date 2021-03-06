#include "pch.h"
#include "commdlg.h"
#include "TextInputCtrl.h"

extern HINSTANCE g_hInst;

#define TEXTINPUTCTRL_CLASSNAME TEXT("TextInputCtrlClassName")
#define TIMERID_CARET 0x100



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

LRESULT CALLBACK CTextInputCtrl::s_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    CTextInputCtrl *ptic;
    switch (message)
    {
        case WM_CREATE:
            SetThis(hwnd, ((CREATESTRUCT *)lParam)->lpCreateParams);
            SetTimer(hwnd, TIMERID_CARET, GetCaretBlinkTime(), NULL);
            GetThis(hwnd)->OnCreate(hwnd, wParam, lParam);
            break;

        case WM_DESTROY:
            ptic = GetThis(hwnd);
            if (ptic)
            {
                ptic->OnDestroy();
            }
            break;

        case WM_SETFOCUS:
            ptic = GetThis(hwnd);
            if (ptic)
            {
                ptic->OnSetFocus(wParam, lParam);
            }
            break;

        case WM_PAINT:
            ptic = GetThis(hwnd);
            hdc = BeginPaint(hwnd, &ps);
            if (ptic)
                ptic->OnPaint(hdc);
            EndPaint(hwnd, &ps);
            break;

        case WM_KEYDOWN:
            ptic = GetThis(hwnd);
            if (ptic)
                ptic->OnKeyDown(wParam, lParam);
            break;

        case WM_LBUTTONDOWN:
            ptic = GetThis(hwnd);
            if (ptic)
                ptic->OnLButtonDown(wParam, lParam);
            break;

        case WM_LBUTTONUP:
            ptic = GetThis(hwnd);
            if (ptic)
                ptic->OnLButtonUp(wParam, lParam);
            break;

        case WM_MOUSEMOVE:
            ptic = GetThis(hwnd);
            if (ptic)
                ptic->OnMouseMove(wParam, lParam);
            break;


        
        case WM_IME_CHAR:
            //
            // wParam is a character of the result string. 
            // if we don't want to receive WM_CHAR message for this character,
            // we should not call DefWindowProc().
            //
            return DefWindowProc(hwnd, message, wParam, lParam);

        case WM_CHAR:
            //
            // wParam is a character of the result string. 
            //
            
            switch ((WCHAR)wParam)
            {
                case 0x08:
                case 0x0a:
                    return 0;
                default:
                    break;
            }

            ptic = GetThis(hwnd);
            if (ptic)
            {
                WCHAR wc[2];
                wc[0] = (WCHAR)wParam;
                wc[1] = L'\0';
                ptic->_editor.InsertAtSelection(wc);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;

        case WM_TIMER:
            if (wParam == TIMERID_CARET)
            {
                ptic = GetThis(hwnd);
                if (ptic)
                {
                    HDC hdc = GetDC(hwnd);
                    ptic->_editor.BlinkCaret(hdc);
                    ReleaseDC(hwnd, hdc);
                }
            }

            break;
    
        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }
    return 0;
}



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnPaint(HDC hdc)
{
    _editor.Render(hdc, &_lfCurrentFont);
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnKeyDown(WPARAM wParam, LPARAM lParam)
{
    UINT nSelStart;
    UINT nSelEnd;
    switch (0xff & wParam)
    {
        case VK_LEFT:
             if (GetKeyState(VK_SHIFT) & 0x80)
             {
                 nSelStart = _editor.GetSelectionStart();
                 nSelEnd = _editor.GetSelectionEnd();
                 if (nSelStart > 0)
                 {
                     _editor.MoveSelection(nSelStart - 1, nSelEnd);
                 }
             }
             else
             {
                 _editor.MoveSelectionPrev();
             }
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_RIGHT:
             if (GetKeyState(VK_SHIFT) & 0x80)
             {
                 nSelStart = _editor.GetSelectionStart();
                 nSelEnd = _editor.GetSelectionEnd();
                 _editor.MoveSelection(nSelStart, nSelEnd + 1);
             }
             else
             {
                 _editor.MoveSelectionNext();
             }
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_UP:
             _editor.MoveSelectionUpDown(TRUE);
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_DOWN:
             _editor.MoveSelectionUpDown(FALSE);
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_HOME:
             _editor.MoveSelectionToLineFirstEnd(TRUE);
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_END:
             _editor.MoveSelectionToLineFirstEnd(FALSE);
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_DELETE:
             nSelStart = _editor.GetSelectionStart();
             nSelEnd = _editor.GetSelectionEnd();
             if (nSelStart == nSelEnd)
             {
                 _editor.DeleteAtSelection(FALSE);
             }
             else
             {
                 _editor.DeleteSelection();
             }
             InvalidateRect(_hwnd, NULL, TRUE);
             break;

        case VK_BACK:
             nSelStart = _editor.GetSelectionStart();
             nSelEnd = _editor.GetSelectionEnd();
             if (nSelStart == nSelEnd)
             {
                 _editor.DeleteAtSelection(TRUE);
             }
             else
             {
                 _editor.DeleteSelection();
             }
             InvalidateRect(_hwnd, NULL, TRUE);
             break;
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnCreate(HWND hwnd, WPARAM wParam, LPARAM lParam)
{
    _editor.SetWnd(hwnd);
    _editor.InitTSF();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnDestroy()
{
    _editor.UninitTSF();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnSetFocus(WPARAM wParam, LPARAM lParam)
{
    _editor.SetFocusDocumentMgr();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnLButtonDown(WPARAM wParam, LPARAM lParam)
{
    POINT pt;
    _uSelDragStart = (UINT)-1;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    if (_editor.MoveSelectionAtPoint(pt))
    {
        InvalidateRect(_hwnd, NULL, TRUE);
        _uSelDragStart = _editor.GetSelectionStart();
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnLButtonUp(WPARAM wParam, LPARAM lParam)
{
    UINT nSelStart = _editor.GetSelectionStart();
    UINT nSelEnd = _editor.GetSelectionEnd();
    POINT pt;
    pt.x = GET_X_LPARAM(lParam);
    pt.y = GET_Y_LPARAM(lParam);
    if (_editor.MoveSelectionAtPoint(pt))
    {
        UINT nNewSelStart = _editor.GetSelectionStart();
        UINT nNewSelEnd = _editor.GetSelectionEnd();
        _editor.MoveSelection(min(nSelStart, nNewSelStart), max(nSelEnd, nNewSelEnd));
        InvalidateRect(_hwnd, NULL, TRUE);
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::OnMouseMove(WPARAM wParam, LPARAM lParam)
{
    if (wParam & MK_LBUTTON)
    {
        POINT pt;
        pt.x = GET_X_LPARAM(lParam);
        pt.y = GET_Y_LPARAM(lParam);
        if (_editor.MoveSelectionAtPoint(pt))
        {
            UINT nNewSelStart = _editor.GetSelectionStart();
            UINT nNewSelEnd = _editor.GetSelectionEnd();
            _editor.MoveSelection(min(_uSelDragStart, nNewSelStart), max(_uSelDragStart, nNewSelEnd));
            InvalidateRect(_hwnd, NULL, TRUE);
        }
    }
}
 
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextInputCtrl::SetFont(HWND hwndParent)
{
    CHOOSEFONT  cf;
    LOGFONT     lf = _lfCurrentFont;

    cf.lStructSize    = sizeof(CHOOSEFONT);
    cf.hwndOwner      = hwndParent;
    cf.lpLogFont      = &lf;
    cf.Flags          = CF_INITTOLOGFONTSTRUCT | CF_SCREENFONTS;
    cf.rgbColors      = RGB(0, 0, 0);
    cf.lCustData      = 0;
    cf.lpfnHook       = NULL;
    cf.lpTemplateName = NULL;
    cf.hInstance      = NULL;
    cf.lpszStyle      = NULL;
    cf.nFontType      = SCREEN_FONTTYPE;
    cf.nSizeMin       = 0;
    cf.nSizeMax       = 0;

    if (ChooseFont(&cf))
    {
        _lfCurrentFont = lf;
        InvalidateRect(_hwnd, NULL, TRUE);
    }

}
