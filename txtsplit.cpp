// txtslip.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "txtsplit.h"
#include <windows.h>
#include <commdlg.h>
#include <stdio.h>
#include <time.h>

#define ID_BUTTON 2
#define ID_TEXTBOX 3
#define ID_FILEBUTTON 4
#define ID_PREFIXTEXTBOX 5

WCHAR filePath[MAX_PATH] = { 0 };
WCHAR prefix[MAX_PATH] = { 0 };
int linesPerFile = 1000;

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void SplitFileByLines(const WCHAR* filePath, const WCHAR* prefix, int linesPerFile);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    WNDCLASS wc = { 0 };
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"FileSplitterClass";

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        wc.lpszClassName,
        L"Text File Splitter",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 500, 200,
        NULL,
        NULL,
        hInstance,
        NULL
    );

    if (hwnd == NULL) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);

    MSG msg = { 0 };
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    static HWND hFileButton, hButton, hTextbox, hPrefixTextbox;

    switch (uMsg) {
    case WM_CREATE: {
        hFileButton = CreateWindow(
            L"BUTTON",
            L"Choose File",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            20, 20, 100, 25,
            hwnd,
            (HMENU)ID_FILEBUTTON,
            NULL,
            NULL
        );

        hPrefixTextbox = CreateWindow(
            L"EDIT",
            L"",
            WS_CHILD | WS_VISIBLE | WS_BORDER,
            130, 20, 200, 25,
            hwnd,
            (HMENU)ID_PREFIXTEXTBOX,
            NULL,
            NULL
        );

        hTextbox = CreateWindow(
            L"EDIT",
            L"1000",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_NUMBER,
            340, 20, 80, 25,
            hwnd,
            (HMENU)ID_TEXTBOX,
            NULL,
            NULL
        );

        hButton = CreateWindow(
            L"BUTTON",
            L"Split Now!",
            WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
            200, 60, 100, 30,
            hwnd,
            (HMENU)ID_BUTTON,
            NULL,
            NULL
        );
        break;
    }

    case WM_COMMAND: {
        if (LOWORD(wParam) == ID_FILEBUTTON) {
            OPENFILENAME ofn;
            ZeroMemory(&ofn, sizeof(ofn));
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hwnd;
            ofn.lpstrFile = filePath;
            ofn.lpstrFile[0] = '\0';
            ofn.nMaxFile = sizeof(filePath);
            ofn.lpstrFilter = L"All\0*.*\0Text\0*.TXT\0";
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

            if (GetOpenFileName(&ofn) == TRUE) {
                SetWindowText(hFileButton, filePath);
                SetWindowText(hPrefixTextbox, ofn.lpstrFileTitle);
            }
        }

        if (LOWORD(wParam) == ID_BUTTON) {
            WCHAR buffer[10];
            GetWindowText(hTextbox, buffer, 10);
            linesPerFile = _wtoi(buffer);

            GetWindowText(hPrefixTextbox, prefix, MAX_PATH);
            if (wcslen(prefix) == 0) {
                wcscpy(prefix, filePath);
            }

            SplitFileByLines(filePath, prefix, linesPerFile);
            MessageBox(hwnd, L"Split Done！", L"Info", MB_OK);
        }
        break;
    }

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    default:
        return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void SplitFileByLines(const WCHAR* filePath, const WCHAR* prefix, int linesPerFile) {
    FILE* file = _wfopen(filePath, L"r");
    if (!file) {
        MessageBox(NULL, L"Can not open", L"Error", MB_OK | MB_ICONERROR);
        return;
    }

    char buffer[1024];
    int fileIndex = 1;
    int lineCount = 0;

    time_t t = time(NULL);
    struct tm tm = *localtime(&t);

    WCHAR outputPath[MAX_PATH];
    swprintf(outputPath, MAX_PATH, L"%s_%d_%04d%02d%02d_%02d%02d%02d.txt",
        prefix, fileIndex, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
        tm.tm_hour, tm.tm_min, tm.tm_sec);
    FILE* outputFile = _wfopen(outputPath, L"w");

    while (fgets(buffer, sizeof(buffer), file)) {
        if (lineCount == linesPerFile) {
            fclose(outputFile);
            lineCount = 0;
            fileIndex++;
            swprintf(outputPath, MAX_PATH, L"%s_%d_%04d%02d%02d_%02d%02d%02d.txt",
                prefix, fileIndex, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday,
                tm.tm_hour, tm.tm_min, tm.tm_sec);
            outputFile = _wfopen(outputPath, L"w");
        }
        fputs(buffer, outputFile);
        lineCount++;
    }

    fclose(outputFile);
    fclose(file);
}
