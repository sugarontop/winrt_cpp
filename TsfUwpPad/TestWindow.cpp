#include "pch.h"
#include "D2DWinrtEx.h"
#include "libsrc/D2DLParam.h"
#include "../libsrc/caret.h"
#include "TextEditor.h"

ITfThreadMgr2* g_pThreadMgr;
TfClientId g_TfClientId;
CTextEditor* g_editor;

///////////////////////////////////////////////////////

HRESULT TestWindow::DefWndProc(D2DWinrtEx* sender, UINT msg, INT_PTR wp, INT_PTR lp)
{
    HRESULT ret = 0;
    switch(msg)
    {
        case WM_LBUTTONDOWN:
        {
            g_editor->SetFocusDocumentMgr();
            V5::Caret::GetCaret().Activate();

            OnLButtonDown(wp,lp);
            IsCapture_ = true;
        }
        break;
        case WM_LBUTTONUP:
        {
            OnLButtonUp(wp, lp);
            IsCapture_ = false;
        }
        break;
        case WM_MOUSEMOVE:
        {
            
            OnMouseMove(wp, lp);
        }
        break;
        case WM_DESTROY:
        {
            OnDestroy();
        }
        break;
        case WM_KEYDOWN:
        {
            V5::KeyEvent& ev = *(V5::KeyEvent*)lp;

            switch(ev.key)
            {
                case VK_RETURN:
                {
                    AddReturn();
                }
                break;
                case VK_HOME:
                {
                    g_editor->MoveSelection(0,0);
                }
                break;
                case VK_END:
                {
                    UINT e = g_editor->GetTextLength();
                    g_editor->MoveSelection(e,e);
                }
                break;
                case VK_LEFT:
                {
                    auto s = max(0, (int)g_editor->GetSelectionStart()-1);
                    g_editor->MoveSelection(s, s);
                }
                break;
                case VK_RIGHT:
                {
                    auto e = g_editor->GetTextLength();
                    auto s = min( e, g_editor->GetSelectionStart()+1);
                    g_editor->MoveSelection(s, s);
                }
                break;
                case VK_BACK:
                {             
                    g_editor->DeleteAtSelection(TRUE);
                }
                break;
                case VK_DELETE:
                {
                    if (g_editor->GetSelectionStart() != g_editor->GetSelectionEnd())
                        g_editor->DeleteSelection();
                    else
                        g_editor->DeleteAtSelection(FALSE);
                }
                break;
            }
        }
        break;
    }
    return ret;
}

HRESULT TestWindow::DefPaintWndProc(D2DWinrtEx* sender, UINT msg, INT_PTR wp, INT_PTR lp)
{
    _ASSERT(msg == WM_PAINT);
    g_editor->Render((D2DContext*)sender->cxt());
    return 0;
}
//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void TestWindow::OnLButtonDown(INT_PTR wParam, INT_PTR lParam)
{
    POINT pt;
    _uSelDragStart = (UINT)-1;


    V5::MouseEvent& ev = *(V5::MouseEvent*)lParam;
    pt.x = ev.pt.x; // GET_X_LPARAM(lParam);
    pt.y = ev.pt.y; //GET_Y_LPARAM(lParam);
    if (g_editor->MoveSelectionAtPoint(pt))
    {
        //InvalidateRect(_hwnd, NULL, TRUE);
        _uSelDragStart = g_editor->GetSelectionStart();
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void TestWindow::OnLButtonUp(INT_PTR wParam, INT_PTR lParam)
{
    UINT nSelStart = g_editor->GetSelectionStart();
    UINT nSelEnd = g_editor->GetSelectionEnd();
    POINT pt;
    V5::MouseEvent& ev = *(V5::MouseEvent*)lParam;
    pt.x = ev.pt.x; // GET_X_LPARAM(lParam);
    pt.y = ev.pt.y; //GET_Y_LPARAM(lParam);
    if (g_editor->MoveSelectionAtPoint(pt))
    {
        UINT nNewSelStart = g_editor->GetSelectionStart();
        UINT nNewSelEnd = g_editor->GetSelectionEnd();
        g_editor->MoveSelection(min(nSelStart, nNewSelStart), max(nSelEnd, nNewSelEnd));
        //InvalidateRect(_hwnd, NULL, TRUE);
    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void TestWindow::OnMouseMove(INT_PTR wParam, INT_PTR lParam)
{
    V5::MouseEvent& ev = *(V5::MouseEvent*)lParam;

    ev.kind = (IsCapture_ ? 1 : 0);

    if (ev.kind == 1)
    {
        POINT pt;        
        pt.x = ev.pt.x; // GET_X_LPARAM(lParam);
        pt.y = ev.pt.y; //GET_Y_LPARAM(lParam);
        if (g_editor->MoveSelectionAtPoint(pt))
        {
            UINT nNewSelStart = g_editor->GetSelectionStart();
            UINT nNewSelEnd = g_editor->GetSelectionEnd();
            g_editor->MoveSelection(min(_uSelDragStart, nNewSelStart), max(_uSelDragStart, nNewSelEnd));
            //InvalidateRect(_hwnd, NULL, TRUE);
        }
    }
}

void TestWindow::AddReturn()
{
    g_editor->InsertAtSelection(L"\n"); // LF
}

void TestWindow::OnDestroy()
{
    g_editor->UninitTSF();
    delete g_editor;
    g_editor = nullptr;

}
///////////////////////////////////////////////////////
BOOL InitTSF();
void OnEntry(D2DWinrtEx* win)
{
    BOOL bl = InitTSF();


}




BOOL InitTSF()
{
	CTextEditor* ed = new CTextEditor();
    g_editor = ed;
	return ed->InitTSF();
}