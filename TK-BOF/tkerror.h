#ifndef _TKERROR_H_
#define _TKERROR_H_

#include <windows.h>
#include "bofdefs.h"

static inline void TkErrorMessage(DWORD dwError, char *buf, int bufSize)
{
	if (!buf || bufSize <= 0)
		return;

	char  *sysBuf = NULL;
	DWORD  ret    = 0;
	int    i      = 0;

	ret = KERNEL32$FormatMessageA(
		0x1300,       // FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_IGNORE_INSERTS
		NULL,
		dwError,
		0,
		(LPSTR) &sysBuf,
		0,
		NULL
	);

	if (ret > 0 && sysBuf != NULL)
	{
		for (i = 0; i < bufSize - 1 && sysBuf[i] != '\0'; i++)
		{
			buf[i] = sysBuf[i];
		}
		buf[i] = '\0';

		while (i > 0 && (buf[i-1] == '\r' || buf[i-1] == '\n' || buf[i-1] == ' '))
		{
			i--;
			buf[i] = '\0';
		}

		KERNEL32$LocalFree((HLOCAL) sysBuf);
	}
	else
	{
		MSVCRT$_snprintf(buf, bufSize, "error %lu", dwError);
	}
}

#endif // _TKERROR_H_
