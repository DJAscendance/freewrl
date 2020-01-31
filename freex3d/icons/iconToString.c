/* convert an xpm file to an ascii string in ARGB for WM_ICON 

this is for X11 Motif windows.

1) convert image to a ppm file.
	e.g.: convert startupImage_32_32.jpg startupImage_32_32.ppm

2) put the name here (in main, variable "fileName"), and compile: 
	gcc iconToString.c -o iconToString
	 to compile it.

3) run ./iconToString

4) output is in the file "icon.h"

which can be compiled in to an application for icon when minimized.

--------------------

look at basicwin.c - from

https://www.google.com/url?sa=t&rct=j&q=&esrc=s&source=web&cd=6&ved=2ahUKEwjJpZuU45nkAhUmh-AKHZXSBeoQFjAFegQIAxAB&url=http%3A%2F%2Fwww.mit.edu%2Fcourse%2F21%2F21.japan_dev%2Fjoshg%2Fxlib-examples%2Fbasicwin%2Fbasic%2Fbasicwin.c&usg=AOvVaw2qXY3KmjGmPxQ0B8gEkrod


*/

#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

unsigned char* readPPM(const char* const fileName, unsigned int* const width, unsigned int* const height, unsigned int* const maximum)
{
	const char p6[] = "P6";
	char header[10];
	unsigned char *pixels = NULL;

	*width = *height = *maximum = 0;

	FILE* fr = fopen(fileName, "rb");
	if (fr == NULL) {
		printf("Unable to open file %s\n", fileName);
		return NULL;
	}

	fscanf(fr, "%9s", header);
	if (strcmp(header, p6))
		printf("Incorrect header %s found\n", header);
	else
		if (fscanf(fr, "%d %d\n%d\n", width, height, maximum) != 3)
			puts("Invalid header values");
		else {
			const unsigned int size = *width * *height * 3;

			if ((pixels = malloc(size)) == NULL)
				puts("Error allocating memory");
			else {
				unsigned int read = 0;

				if ((read = fread(pixels, 1, size, fr)) != size) {
					printf("Error reading binary data. Wanted %i, got %i\n", size, read);
					free(pixels);
					pixels = NULL;
				}
			}
		}

	fclose(fr);
	return pixels;
}

int main()
{
	//char fileName[150] = "startupImage.ppm";
	char fileName[150] = "icon.ppm";
	FILE* fw = fopen("icon.h", "w");
	unsigned int width = 0;
	unsigned int height = 0;
	unsigned int maximum = 0;

	unsigned char* pixelArray = readPPM(fileName, &width, &height, &maximum);

	if (pixelArray) {
		printf("Width: %d\n", width);
		printf("Height: %d\n", height);
		printf("maximum: %d\n", maximum);

		printf ("output in ICON format to icon.h\n");

		fprintf (fw,"unsigned long buffer[] = {\n");
		fprintf (fw,"%d, %d,\n",width, height);
		int i;
		for (i=0; i<width*height*3; i+=3) {
			/* A */ fprintf (fw,"0xFF");
			/* R */ fprintf (fw,"%02x",pixelArray[i]);
			/* G */ fprintf (fw,"%02x",pixelArray[i+1]);
			/* B */ fprintf (fw,"%02x,\n ",pixelArray[i+2]);
		}
		fprintf (fw,"};\n");
		free(pixelArray);
		fclose(fw);
	}

}



