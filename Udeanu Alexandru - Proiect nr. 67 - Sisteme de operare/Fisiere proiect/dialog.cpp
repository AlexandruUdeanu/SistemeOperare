
#include <windows.h>
#include <string>
#include <iostream>
#include "resource.h"

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);						//functii win32
BOOL CALLBACK DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam);
BOOL dlgActive = FALSE;
HWND hwndMain;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
	PSTR szCmdLine, int iCmdShow)
{
	srand(time(NULL));
	HWND hwnd; MSG msg; WNDCLASSEX wndclass;
	
	 
	wndclass.cbSize = sizeof(wndclass);
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0; 
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszMenuName = NULL;
	wndclass.lpszClassName = TEXT("Numere");
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	RegisterClassEx(&wndclass);

	hwnd = CreateWindow("Numere",
		"Numere",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		hInstance,
		NULL);
	SetWindowPos(hwnd, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOACTIVATE);
	ShowWindow(hwnd, SW_HIDE);
	UpdateWindow(hwnd);
	hwndMain = hwnd;

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	static HINSTANCE hInstance;
	switch (iMsg)
	{
	case WM_CREATE:
		hInstance = ((LPCREATESTRUCT)lParam)->hInstance;
		if (!dlgActive) {
			DialogBox(hInstance, MAKEINTRESOURCE(FEREASTRA), 
				hwnd, (DLGPROC)DlgProc);
			PostMessage(hwnd, WM_CLOSE, 0, 0);
		}
		return 0;
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

//se verifica daca un fisier este valid
bool fisierValid(std::string fisier) { 
	DWORD dwAttrib = GetFileAttributes(fisier.c_str());

	return (dwAttrib != INVALID_FILE_ATTRIBUTES &&
		!(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
}

//se copiaza continutul fisierului X in fisierul Y
void copy(HWND hDlg, std::string X, std::string Y) { 
	CopyFile(X.c_str(), Y.c_str(), false);
}

//stergerea fisierul Y
void del(HWND hDlg, std::string Y) { 
	if (fisierValid(Y)) // daca acesta exista
		DeleteFile(Y.c_str());
	else { // daca nu exista, caseta de dialog cu informatia
		std::string text = "Fisierul " + Y + " nu s-a sters deoarece nu exista.";
		MessageBox(hDlg, text.c_str(), "Fisier", MB_OK);
	}
}

// redenumirea fisierul X
void ren(HWND hDlg, std::string X) { 
	std::string::size_type pos = X.rfind('.'); //se cauta ultima pozitie a caracterului "."
	std::string x_file_name = X; //se populeaza numele default cu numele fisierului
	std::string extension = "";

	if (pos != std::string::npos) { //daca acesta exista
		x_file_name = X.substr(0, pos); //numele fisierului este tot pana la punct
		extension = X.substr(pos); //si extensia este restul
	}

	std::string new_file_name = x_file_name + "_tmp" + extension; //se formeaza numele nou
	rename(X.c_str(), new_file_name.c_str()); //si se redenumeste
}

//se verifica daca fieldul X este vid
bool verificaXVid(HWND hDlg, std::string X) {
	if (X.size() == 0) {
		MessageBox(hDlg, "Field-ul X este vid!", "Fisier", MB_OK);
		return true;
	}

	return false;
}

void modify(HWND hDlg, std::string X) { //se modifica atributul de READ_ONLY
	DWORD attributes = GetFileAttributes(X.c_str()); //se preia atributele

	std::string text = "Modifici atributul de readonly din " + X + " ?";
	int confirmare = MessageBox(hDlg, text.c_str(), "Confirmare", MB_YESNO); //dialog confirmare

	switch (confirmare) {
	case IDYES: //daca da, atunci se executa schimbarea
		if (attributes & FILE_ATTRIBUTE_READONLY) //daca are readonly, dezactiveaza atributia
			SetFileAttributes(X.c_str(), FILE_ATTRIBUTE_NORMAL);
		else //iar daca nu, atribuie readonly
			SetFileAttributes(X.c_str(), FILE_ATTRIBUTE_READONLY);
		break;
	}
}

BOOL CALLBACK DlgProc(HWND hDlg, UINT iMsg, WPARAM wParam, LPARAM lParam) {
	char buffer_x[MAX_PATH];
	char buffer_y[MAX_PATH];

	switch (iMsg) {
	case WM_INITDIALOG: //initial se populeaza field-ul de X si Y cu directorul curent a programului
		GetCurrentDirectory(sizeof(buffer_x), buffer_x);
		GetCurrentDirectory(sizeof(buffer_y), buffer_y);
		SetDlgItemText(hDlg, FIELD_X, buffer_x);
		SetDlgItemText(hDlg, FIELD_Y, buffer_y);
		return TRUE;
	case WM_CLOSE:
		dlgActive = FALSE;
		EndDialog(hDlg, 0);
		return TRUE;
	case WM_COMMAND:
		GetDlgItemText(hDlg, FIELD_X, buffer_x, sizeof(buffer_x)); //la fiecare comanda se actualizeaza valoarea field-urilor
		GetDlgItemText(hDlg, FIELD_Y, buffer_y, sizeof(buffer_y));
		switch (LOWORD(wParam)) {
		case BUTON_COPY: 
			if (!verificaXVid(hDlg, buffer_x)) //la fiecare se verifica daca X este vid
				copy(hDlg, buffer_x, buffer_y);
			return TRUE;
		case BUTON_RENAME: 
			if (!verificaXVid(hDlg, buffer_x))
				ren(hDlg, buffer_x);
			return TRUE;
		case BUTON_DELETE: 
			if (!verificaXVid(hDlg, buffer_x))
				del(hDlg, buffer_y);
			return TRUE;
		case BUTON_MODIFY: 
			if(!verificaXVid(hDlg, buffer_x))
				modify(hDlg, buffer_x);
			return TRUE;
		case BUTON_QUIT: 
			PostQuitMessage(0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}
