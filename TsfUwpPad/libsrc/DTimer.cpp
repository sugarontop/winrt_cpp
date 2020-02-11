#include "pch.h"
#include "DTimer.h"

using namespace winrt;
using namespace Windows::UI;
using namespace Windows::UI::Core;
using namespace Windows::System;

extern std::function<void()> g_caret_blick_func;

DTimer::DTimer()
{
	
}
DTimer::~DTimer()
{
	g_caret_blick_func = nullptr;
}

void DTimer::CreatePeriodicTimer(std::function<void(DTimer*)> BlinkFunc )
{
	auto f = [this,BlinkFunc](){ BlinkFunc(this); };

	g_caret_blick_func = f;

}
void DTimer::Cancel()
{
	g_caret_blick_func = [](){};
}