#pragma once
#include <Windows.h>
#include <iostream>

class Window
{
public:
	HWND hwnd; //Where to store the window
	HINSTANCE hinstance; 
	std::string name;
	RECT screen;
	POINT center;
	int width;
	int height;
	bool keys[256];

	int mousex;
	int mousey;
	int deltaX;
	int deltaY;
	bool dirty = false;

	bool mouseButtons[3];
	int mouseWheel;
	bool firstMouse = true;

	//LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	void create(std::string window_name, int _window_width, int _window_height, int window_x = 0, int window_y = 0);
	void processMessages();
	void LockCursorToWindow();
	void CenterCursor();
	void updateMouse(int x, int y);
	bool keyPressed(int key)
	{
		return keys[key];
	}
};
