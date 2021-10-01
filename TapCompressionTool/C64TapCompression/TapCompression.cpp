#include "stdafx.h"
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

// based on this:
//https://github.com/charcole/C64Tape/blob/master/c64convert.c

using namespace std;

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;

#define BITS_INDEX   8 //  8,9 or 10  good value is 10
#define BITS_REPEAT  (16 - BITS_INDEX)
#define REPEAT_LIMIT (1<<BITS_REPEAT)

#define WINDOW_SIZE  (1<<BITS_INDEX)  

#define READ_BUFFER_SIZE (64)
#define FAT_BUF_SIZE (128)

u8 window[WINDOW_SIZE];

u8* LZLikeDecode(u8 *out, u8 *in, u8 *end) {
	u16 head = 0;
	while (in < end) {
		u8 a = *(in++);
		u8 b = *(in++);
 		if (a == 0) { 
			// no special repetition, recreate history
			window[(head++)&(WINDOW_SIZE - 1)] = b; 
			*(out++) = b;  
		}
		else {
			u16 idx = head - 1 - b + WINDOW_SIZE;
			for (int i = 0; i <= a; i++) {
				u8 v = window[(idx + i)&(WINDOW_SIZE - 1)];
				window[(head++)&(WINDOW_SIZE - 1)] = v;
				*(out++) = v;
			}
		}
	}
	return out;
}

u8* LZLikeEncode(u8 *out, u8 *in, u8 *end) {
	u8* pcurrentBlockStart = in;
	while (pcurrentBlockStart < end) {
		u8 offset = *pcurrentBlockStart;
		u16 repetitionLength = 1;
		for (int wi = WINDOW_SIZE-1; wi > 0;  wi--) {
			u8* psearchIndex = pcurrentBlockStart;
			if (psearchIndex > in + wi) {
				int len = 0;
				// Does the past have this same sequence, hunt for the longest.
				while (*psearchIndex == *(psearchIndex - 1 - wi)  && len < REPEAT_LIMIT && psearchIndex < end) {
					len++;
					psearchIndex++;
				}
				if (len > repetitionLength) {
					repetitionLength = len;    // found a sequence in the past
					offset = wi;				// distance we looked back in bytes
					if (len == REPEAT_LIMIT)
						break;
				}
			}
		}
		// - No sequence:    writes [0],[byte]
		// - Found sequence: writes [8 repetitions bits],[8 offset bits]  
		// Remember an Arduino has to decode all this with its 2KB of dynamic memory to run the whole microcontroller.
		// Even just using one extra bit on offset from repetitions, so 9 bits will eat 512bytes (and 10bits will eat 1024bytes)
		// (8 is just doable at 256bytes without giving the Tapuino code a big facelift)
		*(out++) = repetitionLength - 1;  // how many repeating occurrences discovered
		*(out++) = offset;				  // where to start history playback
		pcurrentBlockStart += repetitionLength;
	}
	return out;
}

// debug, vs config props, cmd args use :  "C:\\Users\\Admin\\Desktop\\New folder\\"

#include <windows.h>
int main(int argc, char **argv)
{
	string folder = "";
	if (argc == 2) {
		folder = argv[1];  
	}
	else {
		printf("Help : This tool finds and compresses all TAP files inside a given folder\n");
		printf(" - Orignal TAP files will not deleted\n");
		printf(" - Compressed files will be created with a ZAP file extension\n\n");
		printf("Example usage:\n");
		printf("  TAPCOMPRESSION [.\\]                          (tools run path)\n");
		printf("  TAPCOMPRESSION [yourTaps\\]                   (relative folder)\n");
		printf("  TAPCOMPRESSION [c:\\data\\c64\\yourTaps\\]       (absolute path)\n");
		getchar();
		return 0;
	}

	WIN32_FIND_DATAA ffd;

	printf("Packing all tap files found here: %s\n\n" , folder.c_str());

	HANDLE  hFind = FindFirstFileA((folder + "*.tap").c_str(), &ffd);

	if (hFind == INVALID_HANDLE_VALUE) {
		printf("FindFirstFileA: INVALID_HANDLE_VALUE");
		return 0;
	}

	vector<string> files;
	do {
		if (ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			//	printf("  %s   <DIR>\n", ffd.cFileName);
		}
		else {
			string fileName = folder + ffd.cFileName; 
			files.push_back(fileName);
		}
	} while (FindNextFileA(hFind, &ffd) != 0);
		

	int totalOriginal = 0;
	int totalCompressed = 0;
	int passed = 0;
	for (vector<string>::iterator it = files.begin(); it != files.end(); it++)
	{
		string fileName = *it; 
		std::string inFilename = fileName.substr(fileName.find_last_of("/\\") + 1);
		printf("%d) %s found, ", passed+1 , inFilename.c_str() );

		FILE *f;
		fopen_s(&f, fileName.c_str(), "rb");
		if (f){
			fseek(f, 0, SEEK_END);
			long size = ftell(f);
			fseek(f, 0, SEEK_SET);
			u8 *data = (u8*)malloc(size);
			fread(data, 1, size, f);
			fclose(f);
			u8 *out = (u8*)malloc(2 * size); // worst case
			u8 *eout = LZLikeEncode(out, data, data + size);
			printf("packed %d -> %d bytes, ", (int)size, (int)(eout - out));

			totalOriginal += size;
			totalCompressed += (int)(eout - out);
			//----------
			string newfilename = fileName.substr(0, fileName.find_last_of('.')) + ".zap";

			std::string base_filename = newfilename.substr(newfilename.find_last_of("/\\") + 1);
			printf("%s saved, ", base_filename.c_str());
		

//		if (remove(newfilename.c_str())==-1)
//			printf("Could not delete %s\n" , newfilename.c_str());
//		else
//			printf("Deleted %s\n", newfilename.c_str());

			FILE *g;
			fopen_s(&g, newfilename.c_str(), "wb");
			if (g)	{
				fwrite(out, 1, eout - out, g);
				fclose(g);
			}
			else {
				printf("Couldn't open '%s' for write\n", argv[2]);
				return 1;
			}

			u8 *dest = (u8*)malloc(size);
			u8 *edest = LZLikeDecode(dest, out, eout);
			if (memcmp(dest, data, size) != 0) {
				printf("Decompress error %d/%d\n", (int)(edest - dest), (int)size);
				return 1;
			}
			else {
				passed++;
				printf("verify Passed\n");
			}

			free(data);
			free(dest);
		}
	}
	
	printf("\nStats:\n%d bytes total for original taps\n", totalOriginal);
	printf("%d bytes total for compressed\n", totalCompressed);
	printf("saving %d bytes, %dKB, %dMB (%d files)\n", totalOriginal - totalCompressed, (totalOriginal - totalCompressed)/1024, (totalOriginal - totalCompressed)/1024/1024,passed);

	
//	getchar();
}

