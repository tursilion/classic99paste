// classic99paste.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <Windows.h>

#define ID_FILE_RESET					40019
#define ID_EDITPASTE                    40022
#define ID_CPUTHROTTLING_CPUOVERDRIVE	40068

int _tmain(int argc, _TCHAR* argv[])
{
	if (argc < 2) {
		printf("classic99paste [-reset|-resetOD] <string>\n");
		printf("Passes in the specified string as a paste string to Classic99\n");
		printf("If '-reset' is the first argument, does a cold reset first\n");
		printf("If '-resetOD' is passed, the system is set to CPU overdrive after reset\n");
		printf("The rest of the line is taken as a paste string.\n");
		printf("\\n = enter, \\\\ = real slash\n");
		return -1;
	}

	HWND hWnd = FindWindow("TIWndClass", NULL);
	if (NULL == hWnd) {
		printf("Failed to find Classic99 window!\n");
		return 1;
	}

	int nFirst = 1;

	if (*argv[1]=='-') {
		nFirst++;
		if (0 == _stricmp(argv[1], "-reset")) {
			// just ask Classic99 to reset
			SendMessage(hWnd, WM_COMMAND, ID_FILE_RESET, 0);
			// give it a moment...
			Sleep(500);
		} else if (0 == _stricmp(argv[1], "-resetOD")) {
			// just ask Classic99 to reset
			SendMessage(hWnd, WM_COMMAND, ID_FILE_RESET, 0);
			// give it a moment...
			Sleep(500);
			// and go to overdrive
			SendMessage(hWnd, WM_COMMAND, ID_CPUTHROTTLING_CPUOVERDRIVE, 0);
		} else {
			printf("Unknown command argument.\n");
			return 2;
		}
	}

	char *pOutStr = (char*)malloc(16384);
	if (NULL == pOutStr) {
		printf("Can't allocate memory\n");
		return 3;
	}
	int nPtr = 0;

	while (nFirst < argc) {
		int nSize = strlen(argv[nFirst]);
		if (nPtr+nSize > 16382) {
			printf("Command string too long\n");
			return 4;
		}
		memcpy(&pOutStr[nPtr], argv[nFirst], nSize);
		nPtr+=nSize;
		if (++nFirst < argc) {
			pOutStr[nPtr++]=' ';
		}
	}
	pOutStr[nPtr++]='\0';

	// parse the string and replace \n with enter, any other \ character is the actual character
	for (int idx=0; idx<nPtr; idx++) {
		if (pOutStr[idx] != '\\') continue;
		if (pOutStr[idx+1] == '\0') break;	// end of string
		if (pOutStr[idx+1] == 'n') {
			// this is it!
			pOutStr[idx] = '\n';
			memmove(&pOutStr[idx+1], &pOutStr[idx+2], nPtr-(idx+1));
			nPtr--;
		} else {
			// it's something benign, replace it
			memmove(&pOutStr[idx], &pOutStr[idx+1], nPtr-idx);
			nPtr--;
		}
	}

	if (OpenClipboard(NULL)) {
        // mandatory on later versions of Windows
        EmptyClipboard();

		HGLOBAL hMem = GlobalAlloc(GMEM_SHARE | GMEM_MOVEABLE, nPtr+1);
		if (NULL == hMem) {
			free(pOutStr);
			printf("Can't allocate clipboard memory\n");
			return 5;
		}
		LPSTR pTxt = (LPSTR)GlobalLock(hMem);
		memcpy(pTxt, pOutStr, nPtr+1);
		GlobalUnlock(hMem);
        free(pOutStr);
		bool bRet = (SetClipboardData(CF_TEXT, hMem) != NULL);
		CloseClipboard();
		if (!bRet) {
			printf("Failed to set clipboard data\n");
			return 6;
		}
	} else {
		printf("Failed to open clipboard.\n");
		return 7;
	}

	// tell Classic99 to paste - sometimes getting access denied
    // under Win10, maybe the CloseClipboard is taking longer?
    // Maybe virus checker is interfering...
    Sleep(100);
	PostMessage(hWnd, WM_COMMAND, ID_EDITPASTE, 0);

	return 0;

}

 