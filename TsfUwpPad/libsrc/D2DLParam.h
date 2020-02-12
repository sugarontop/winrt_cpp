#pragma once


namespace V5
{

struct MouseEvent
{
	FPointF pt;
	int bLeft; // 1:lbutton, -1:rbutton
	int wheel_delta;
	int kind;
};

struct KeyEvent
{
	int key;
	bool IsPressShift;
	bool IsPressControl;

};
struct SizeChangeEvent
{
	FSizeF sz;
};
struct CharEvent
{
	UINT keycode;
};
struct AddCharEvent
{
	const WCHAR* ch;
	UINT cch;


};

};