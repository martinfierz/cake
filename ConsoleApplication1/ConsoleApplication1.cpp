// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <conio.h>
#include <stdio.h>


int main()
{
	//return 0;
	int c1 = 1, c2 = 128;
	signed short s;
	unsigned short u; 
	int i = 0;

	s = c1 | (c2 << 8);
	u = c1 | (c2 << 8); 
	//printf("\nhello world");
	printf("\n s is %i", s);
	printf("\n u is %i", u);
	_getch();
	
		
}

