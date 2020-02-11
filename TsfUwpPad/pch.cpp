#include "pch.h"


void Trace(LPCWSTR fm, ...)
{
	if (fm)
	{
		va_list ap;
		va_start(ap, fm);
		size_t len = _vscwprintf(fm, ap) + 1;


		WCHAR* b = new WCHAR[len];
		WCHAR* buffer = b;


		int nWritten = _vswprintf_l(buffer, len, fm, 0, ap);

		if (nWritten > 0)
		{
			buffer[len - 1] = 0;

		}

		va_end(ap);

		::OutputDebugString(buffer);

		delete[] b;
	}
}