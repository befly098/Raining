#pragma once
#include<Windows.h>
#include<math.h>
#include<stdio.h>

int coord_x(HANDLE consoleHandler);
int coord_y(HANDLE consoleHandler);

int coord_x()
{
	//POINT getter;
	CONSOLE_SCREEN_BUFFER_INFO curInfo;

	//GetCursorPos(&getter);
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);

	return curInfo.dwCursorPosition.X;
}

int coord_y()
{
	//POINT getter;
	CONSOLE_SCREEN_BUFFER_INFO curInfo;

	//GetCursorPos(&getter);
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);

	return curInfo.dwCursorPosition.Y;
}