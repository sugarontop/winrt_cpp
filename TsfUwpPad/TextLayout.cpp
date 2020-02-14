#include "pch.h"
#include "TextLayout.h"
#include "libsrc/D2DContext.h"
#include "libsrc/caret.h"

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

#define CR 13
#define LF 10
#define ROW_HEIGHT 20

BOOL CTextLayout::Layout(D2DContext* cxt, const WCHAR *psz,  UINT nCnt)
{
    Clear();

    // Count Line
    UINT i = 0;
    BOOL bNewLine = TRUE;
    _nLineCnt = 0;
    for (i = 0; i < nCnt; i++)
    {
        switch (psz[i])
        {
            case CR:
            case LF:
                if (bNewLine)
                    _nLineCnt++;

                bNewLine = TRUE;
                break;
            default:
                if (bNewLine)
                    _nLineCnt++;
                bNewLine = FALSE;
                break;
        }
    }

    _prgLines = (LINEINFO *)LocalAlloc(LPTR, _nLineCnt * sizeof(LINEINFO));
    if (!_prgLines)
        return FALSE;

    // Count character of each line.
    bNewLine = TRUE;
    int nCurrentLine = -1;
    for (i = 0; i < nCnt; i++)
    {
        switch (psz[i])
        {
            case CR:
            case LF:
                if (bNewLine)
                {
                    nCurrentLine++;
                    _prgLines[nCurrentLine].nPos = i;
                    _prgLines[nCurrentLine].nCnt = 0;
                }

                bNewLine = TRUE;
                break;
            default:
                if (bNewLine)
                {
                    nCurrentLine++;
                    _prgLines[nCurrentLine].nPos = i;
                    _prgLines[nCurrentLine].nCnt = 1;
                }
                else
                {
                    _prgLines[nCurrentLine].nCnt++;
                }
                bNewLine = FALSE;
                break;
        }
    }

    //TEXTMETRIC tm;
    //GetTextMetrics(hdc, &tm);
    //_nLineHeight = tm.tmHeight + tm.tmExternalLeading;

    _nLineHeight = ROW_HEIGHT;

    V5::D2DContext* pcxt = (V5::D2DContext *)cxt;
    
    float w = 1920;
    float h = 1680;

    POINT ptCurrent;
    ptCurrent.x = 0;
    ptCurrent.y = 0;

    int char_off = 0;

    std::vector<RECT> rcs;

    layouts_.resize(_nLineCnt);
    UINT j;
    // Get the rectangle of each characters.
    for (i = 0; i < _nLineCnt; i++)
    {
        _prgLines[i].prgCharInfo = NULL;
        j = 0;

        if (_prgLines[i].nCnt)
        {
            _prgLines[i].prgCharInfo = (CHARINFO *)LocalAlloc(LPTR, _prgLines[i].nCnt * sizeof(CHARINFO));
            if (!_prgLines[i].prgCharInfo)
                return FALSE;

            IDWriteTextLayout* layout;
            if (FAILED(pcxt->cxtt.wfactory->CreateTextLayout(psz+ char_off, _prgLines[i].nCnt, pcxt->cxtt.textformat, w, h, &layout)))
                return FALSE;
                
            layouts_[i] = layout;

            
            POINT ptPrev = ptCurrent;
            for (j = 0; j < _prgLines[i].nCnt; j++)
            {
                //SIZE size;
                //GetTextExtentPoint32(hdc, psz+_prgLines[i].nPos, j + 1, &size);

                DWRITE_HIT_TEST_METRICS tm;
                float x1 = 0, y1 = 0;
                layout->HitTestTextPosition(j, false, &x1, &y1, &tm); 

                ptCurrent.x = x1 + tm.width;
                _prgLines[i].prgCharInfo[j].rc.left = ptPrev.x;
                _prgLines[i].prgCharInfo[j].rc.top = ptPrev.y;
                _prgLines[i].prgCharInfo[j].rc.right = ptCurrent.x;
                _prgLines[i].prgCharInfo[j].rc.bottom = ptPrev.y + _nLineHeight;

                ptPrev = ptCurrent;

                rcs.push_back(_prgLines[i].prgCharInfo[j].rc);
            }
        }

        char_off += _prgLines[i].nCnt;
        char_off++; // LF分


        // LFは幅=0として
        {
            auto it = rcs.end()-1;
            RECT rc = *it;
            rc.left = rc.right;
            rc.top = ptCurrent.y;
            rc.bottom = rc.top + _nLineHeight;
            if (j == 0) {
                rc.left = rc.right = 0;
            }

            rcs.push_back(rc);
        }


        ptCurrent.x = 0;
        ptCurrent.y += _nLineHeight;
    }

    V5::Caret::GetCaret().rcs = rcs;




    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextLayout::Render(D2DContext* pcxt, const WCHAR *psz,  UINT nCnt, UINT nSelStart, UINT nSelEnd)
{
    V5::D2DContext& cxt = *(V5::D2DContext*)pcxt;


    V5::FPointF pt(0,0);
    for(auto& it : layouts_ )
    {
        if ( it != nullptr )
            cxt.cxt->DrawTextLayout(pt, it, cxt.black );

        pt.y += ROW_HEIGHT;
    }


    //
    // render char rects
    //
    //for(auto& it : V5::Caret::GetCaret().rcs )
    //{        
    //    cxt.cxt->DrawRectangle( V5::FRectF(it), cxt.gray );
    //}



    //
    // render selected area
    //
    for(UINT i = nSelStart; i < nSelEnd; i++ )
    {
        auto rc = V5::Caret::GetCaret().rcs[i];

        cxt.cxt->FillRectangle(V5::FRectF(rc), cxt.halftoneRed );

    }

    //
   // reder LF
   //
    for (UINT i = 0; i < V5::Caret::GetCaret().rcs.size(); i++)
    {
        auto rc = V5::Caret::GetCaret().rcs[i];

        if (rc.left == rc.right)
        {
            V5::FRectF rcf = rc;

            rcf.left++;
            rcf.right = rcf.left + 40;

            cxt.cxt->DrawText(L"⏎", 1, cxt.cxtt.textformat, rcf, cxt.green);
        }
    }

    //
    // EOF
    //
    V5::FRectF rcf;
    if (!V5::Caret::GetCaret().rcs.empty())
    {
        auto it = V5::Caret::GetCaret().rcs.end() - 1;
        rcf = *it;
        if (rcf.left == rcf.right)
        {
            rcf.Offset(0, rcf.Height());
            rcf.left = rcf.right = 0;
        }

        rcf.left++;
        rcf.right = rcf.left + 40;
    }
    else
    {
        rcf.SetRect(1, 0, 41, ROW_HEIGHT);
    }

    cxt.cxt->DrawText(L"[EOF]", 5, cxt.cxtt.textformat, rcf, cxt.green);

    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

BOOL CTextLayout::RectFromCharPos(UINT nPos, RECT *prc)
{
    memset(prc, 0, sizeof(*prc));
    for (UINT i = 0; i < _nLineCnt; i++)
    {
        if (nPos < _prgLines[i].nPos)
           continue;

        if (nPos >= _prgLines[i].nPos + _prgLines[i].nCnt)
        {
           if (((nPos -_prgLines[i].nPos) > 0) && (nPos == _prgLines[i].nPos + _prgLines[i].nCnt))
           {
               *prc = _prgLines[i].prgCharInfo[nPos - _prgLines[i].nPos - 1].rc;
               prc->left = prc->right;
               return TRUE;
           }
           continue;
        }
        
        *prc = _prgLines[i].prgCharInfo[nPos - _prgLines[i].nPos].rc;
        return TRUE;
    }
    
    prc->top = _nLineCnt * _nLineHeight;
    prc->bottom = prc->top +  _nLineHeight;
    return TRUE;
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

bool PtInRect(RECT* rc, POINT pt)
{
    return  ( rc->left <= pt.x && pt.x <= rc->right && rc->top <= pt.y  && pt.y <= rc->bottom );
}

UINT CTextLayout::CharPosFromPoint(POINT pt)
{
    for (UINT i = 0; i < _nLineCnt; i++)
    {
        for (UINT j = 0; j < _prgLines[i].nCnt; j++)
        {
            if (PtInRect(&_prgLines[i].prgCharInfo[j].rc, pt))
            {
                int nWidth = _prgLines[i].prgCharInfo[j].GetWidth();
                if (pt.x > _prgLines[i].prgCharInfo[j].rc.left + (nWidth * 3 / 4))
                {
                    return _prgLines[i].nPos + j + 1;
                }
                return _prgLines[i].nPos + j;
            }
        }
    }
    return (UINT)(-1);
}

//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

UINT CTextLayout::FineFirstEndCharPosInLine(UINT uCurPos, BOOL bFirst)
{
    for (UINT i = 0; i < _nLineCnt; i++)
    {
        if ((_prgLines[i].nPos <= uCurPos) &&
            (_prgLines[i].nPos + _prgLines[i].nCnt >= uCurPos))
        {
            if (bFirst)
            {
                return _prgLines[i].nPos;
            }
            else
            {
                return _prgLines[i].nPos + _prgLines[i].nCnt;
            }
        }
    }
    return (UINT)(-1);
}


//----------------------------------------------------------------
//
//
//
//----------------------------------------------------------------

void CTextLayout::Clear()
{
    if (_prgLines)
    {
        for (UINT i = 0; i < _nLineCnt; i++)
        {
            if (_prgLines[i].prgCharInfo)
            {
                LocalFree(_prgLines[i].prgCharInfo);
            }
        }
        LocalFree(_prgLines);
    }
    _nLineCnt = 0;

    for(auto& it : layouts_)
    {
        if ( it )
            it->Release();
    }
    layouts_.clear();
}
