#include "pch.h"
#include "TextEditor.h"
#include "libsrc/caret.h"

extern ITfThreadMgr2 *g_pThreadMgr;
extern TfClientId g_TfClientId;


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::MoveSelection(UINT nSelStart, UINT nSelEnd)
{
    UINT nTextLength = GetTextLength();
    if (nSelStart >= nTextLength)
        nSelStart = nTextLength;

    if (nSelEnd >= nTextLength)
        nSelEnd = nTextLength;

    _nSelStart = nSelStart;
    _nSelEnd = nSelEnd;


TRACE(L"%d,%d\n", _nSelStart, _nSelEnd );

    _pTextStore->OnCaret1Caret2ChangeFromEditor();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::MoveSelectionNext()
{
    UINT nTextLength = GetTextLength();
    if (_nSelEnd < nTextLength)
       _nSelEnd++;

    _nSelStart = _nSelEnd;
    _pTextStore->OnCaret1Caret2ChangeFromEditor();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::MoveSelectionPrev()
{
    if (_nSelStart > 0)
        _nSelStart--;

    _nSelEnd = _nSelStart;
    _pTextStore->OnCaret1Caret2ChangeFromEditor();
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::MoveSelectionAtPoint(POINT pt)
{
    BOOL bRet = FALSE;
    UINT nSel = _layout.CharPosFromPoint(pt);
    if (nSel != (UINT)-1)
    {
        MoveSelection(nSel, nSel);
        bRet = TRUE;
    }
    return bRet;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::MoveSelectionUpDown(BOOL bUp)
{
    RECT rc;
    if (!_layout.RectFromCharPos(_nSelStart, &rc))
        return FALSE;

    POINT pt;
    pt.x = rc.left;
    if (bUp)
    {
        pt.y = rc.top - ((rc.bottom - rc.top) / 2);
        if (pt.y < 0)
            return FALSE;
    }
    else
    {
        pt.y = rc.bottom + ((rc.bottom - rc.top) / 2);
    }

    return MoveSelectionAtPoint(pt);
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::MoveSelectionToLineFirstEnd(BOOL bFirst)
{
    BOOL bRet = FALSE;
    UINT nSel;

    if (bFirst)
    {
        nSel = _layout.FineFirstEndCharPosInLine(_nSelStart, TRUE);
    }
    else
    {
        nSel = _layout.FineFirstEndCharPosInLine(_nSelEnd, FALSE);
    }

    if (nSel != (UINT)-1)
    {
        MoveSelection(nSel, nSel);
        bRet = TRUE;
    }
    return bRet;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::InsertAtSelection(LPCWSTR psz)
{
    LONG lOldSelEnd = _nSelEnd;
    if (!RemoveText(_nSelStart, _nSelEnd - _nSelStart))
        return FALSE;

    if (!InsertText(_nSelStart, psz, wcslen(psz)))
        return FALSE;

    _nSelStart += wcslen(psz);
    _nSelEnd = _nSelStart;

    _pTextStore->OnTextChangeFromEditor(_nSelStart, lOldSelEnd, _nSelEnd);
    _pTextStore->OnCaret1Caret2ChangeFromEditor();
    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::DeleteAtSelection(BOOL fBack)
{
    if (!fBack && (_nSelEnd < GetTextLength()))
    {
        if (!RemoveText(_nSelEnd, 1))
            return FALSE;

        _pTextStore->OnTextChangeFromEditor(_nSelEnd, _nSelEnd + 1, _nSelEnd);
    }

    if (fBack && (_nSelStart > 0))
    {
        if (!RemoveText(_nSelStart - 1, 1))
            return FALSE;

        _nSelStart--;
        _nSelEnd = _nSelStart;

        _pTextStore->OnTextChangeFromEditor(_nSelStart, _nSelStart + 1, _nSelStart);
        _pTextStore->OnCaret1Caret2ChangeFromEditor();
    }

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::DeleteSelection()
{
    ULONG nSelOldEnd = _nSelEnd;
    RemoveText(_nSelStart, _nSelEnd - _nSelStart);

    _nSelEnd = _nSelStart;

    _pTextStore->OnTextChangeFromEditor(_nSelStart, nSelOldEnd, _nSelStart);
    _pTextStore->OnCaret1Caret2ChangeFromEditor();

    return TRUE;
}



//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::Render(D2DContext* cxt)
{
 //   HFONT hFont = CreateFontIndirect(plf);

 //   if (hFont)
 //   {
 //       HFONT hFontOrg = (HFONT)SelectObject(hdc, hFont);

        V5::Caret::GetCaret().blink_pos_ = _nSelEnd;

        _layout.Layout(cxt, GetTextBuffer(), GetTextLength());
        _layout.Render(cxt, GetTextBuffer(), GetTextLength(), _nSelStart, _nSelEnd);

//        SelectObject(hdc, hFontOrg);
//        DeleteObject(hFont);
//    }
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::InitTSF()
{

    _pTextStore = new CTextStore(this);
    if (!_pTextStore)
    {
        return FALSE;
    }

    if (FAILED(::CoCreateInstance(CLSID_TF_ThreadMgr, nullptr, CLSCTX_INPROC_SERVER, IID_ITfThreadMgr2, (void**)&g_pThreadMgr)))
    {
        return FALSE;
    }

    if (FAILED(g_pThreadMgr->CreateDocumentMgr(&_pDocumentMgr)))
    {
        return FALSE;
    }

    if (FAILED(_pDocumentMgr->CreateContext(g_TfClientId, 0, _pTextStore, &_pInputContext, &_ecTextStore)))
    {
        return FALSE;
    }

    if (FAILED(_pDocumentMgr->Push(_pInputContext)))
    {
        return FALSE;
    }


    

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextEditor::UninitTSF()
{
    if (_pDocumentMgr)
    {
        _pDocumentMgr->Pop(TF_POPF_ALL);
    }

    if (_pInputContext)
    {
        _pInputContext->Release();
        _pInputContext = NULL;
    }

    if (_pDocumentMgr)
    {
        _pDocumentMgr->Release();
        _pDocumentMgr = NULL;
    }

    if (_pTextStore)
    {
        _pTextStore->Release();
        _pTextStore = NULL;
    }

    return TRUE;
}


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextEditor::SetFocusDocumentMgr()
{
    if (_pDocumentMgr)
    {
        g_pThreadMgr->SetFocus(_pDocumentMgr);
    }
}

