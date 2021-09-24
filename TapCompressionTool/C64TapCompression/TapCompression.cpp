
#include "stdafx.h"

#include <string>
#include <vector>

#include <stdio.h>
#include <stdlib.h>

// based on this:
//https://github.com/charcole/C64Tape/blob/master/c64convert.c

using namespace std;

#define BITS_INDEX   8 //  8,9 or 10  good value is 10
#define BITS_REPEAT  (16 - BITS_INDEX)
#define WINDOW_SIZE  (1<<BITS_INDEX)  
#define SHIFT  (8 - BITS_REPEAT)
#define REPEAT_LIMIT (1<<(8 - SHIFT))
#define REPEAT_MASK  (REPEAT_LIMIT - 1) 

typedef signed char s8;
typedef unsigned char u8;
typedef signed short s16;
typedef unsigned short u16;

static u8 window[WINDOW_SIZE];


using namespace std;

FILE* fileWrite;
FILE* fileRead;

//--------------
u16 head = 0;
#define READ_BUFFER_SIZE (64)
u8 ReadBuffer[READ_BUFFER_SIZE];
u16 length;
u16 offset;
//u16 amount_read = 0;
//u8 read_count = 0;

void LZLikeReset() {
	head = 0;
	length = 0;
//	amount_read = 0;
//	read_count = 0;
	fopen_s(&fileWrite, "C:\\Users\\Admin\\Desktop\\New folder\\temp.tap", "wb");
	fopen_s(&fileRead, "C:\\Users\\Admin\\Desktop\\New folder\\180.zap", "rb");
	memset(window, 0, READ_BUFFER_SIZE);
}

uint8_t Decode(uint8_t* dataPtr) {
	if (length == 0) {

		fread(ReadBuffer, 1, 2, fileRead);

		uint8_t a = dataPtr[0];
		uint8_t b = dataPtr[1];
		if (a == 0) {
			window[(head++)&(WINDOW_SIZE - 1)] = b;
			return b;
		}
		else {
		    offset = head - 1 - b + WINDOW_SIZE;
			length=a+1;
			offset += length - 1;
		}
	}
	length--;
	uint8_t v = window[(offset - length)&(WINDOW_SIZE - 1)];
	window[(head++)&(WINDOW_SIZE - 1)] = v;
	return v;
}


uint16_t ReadDecodeAmount(uint8_t* dataPrt, uint16_t bytes) {
	for (uint16_t i = 0; i<bytes; i++) {
		dataPrt[i] = Decode(&ReadBuffer[0]);
	}
	return bytes;
}



//-------------

void LZLikeDecodeToFile(const char* fileName)
{
	FILE* fileRead;
	fopen_s(&fileRead, fileName , "rb");
	fseek(fileRead, 0, SEEK_END);
	long size = ftell(fileRead);
	fseek(fileRead, 0, SEEK_SET);
	FILE* fileWrite;
	fopen_s(&fileWrite,"C:\\Users\\Admin\\Desktop\\New folder\\temp.tap" , "wb");
	u16 head = 0;
	for (int i=0 ;i<size/2; i++) {
		 u8 a = fgetc(fileRead);		 
		 u8 b = fgetc(fileRead);
		if (a == 0) {
			window[(head++)&(WINDOW_SIZE - 1)] = b;
			fputc(b, fileWrite);
		}
		else{
			u16 idx = head - 1 - (b + ((a >> (8 - SHIFT)) << 8)) + WINDOW_SIZE;
			a &= REPEAT_MASK;
			for (int i = 0; i <= a; i++) {
				u8 v = window[(idx + i)&(WINDOW_SIZE - 1)];
				window[(head++)&(WINDOW_SIZE - 1)] = v;
				fputc(v, fileWrite);
			}
		}
	}
	fclose(fileWrite);
	fclose(fileRead);
}


#define FAT_BUF_SIZE 128

u8 g_fat_buffer[FAT_BUF_SIZE];

void LZLikeDecodeToFile2(const char* fileName)
{

	memset(g_fat_buffer,0, FAT_BUF_SIZE);
	memset(window, 0, FAT_BUF_SIZE);

	FILE* fileRead;
	fopen_s(&fileRead, fileName, "rb");
	fseek(fileRead, 0, SEEK_END);
	long size = ftell(fileRead);
	fseek(fileRead, 0, SEEK_SET);
	FILE* fileWrite;
	fopen_s(&fileWrite, "C:\\Users\\Admin\\Desktop\\New folder\\temp.tap", "wb");


	printf("FAT_BUF_SIZE %d\n", FAT_BUF_SIZE);
	printf("size %d\n", size);
	printf("size/FAT_BUF_SIZE %d\n", size/ FAT_BUF_SIZE);

	uint32_t head = 0;
	for (uint32_t i = 0; i<size / FAT_BUF_SIZE; i++) {

		size_t r = fread(&g_fat_buffer[0], FAT_BUF_SIZE , 1, fileRead);

		for (uint8_t k = 0; k<FAT_BUF_SIZE; k += 2) {

			u8 a = g_fat_buffer[k + 0];
			u8 b = g_fat_buffer[k + 1];

			if (a == 0) {
				window[(head++)&(WINDOW_SIZE - 1)] = b;
				size_t r = fwrite(&b, 1,1,fileWrite);
			}
			else {
				uint16_t idx = head - 1 - (b + ((a >> (8 - SHIFT)) << 8)) + WINDOW_SIZE;
				a &= REPEAT_MASK;
				for (uint16_t i = 0; i <= a; i++) {
					uint8_t v = window[(idx + i)&(WINDOW_SIZE - 1)];
					window[(head++)&(WINDOW_SIZE - 1)] = v;
					size_t r = fwrite(&v, 1, 1, fileWrite);
				}
			}
		}
	}


	u16 leftover = size % FAT_BUF_SIZE;
	if (leftover) {
		
		size_t r = fread(&g_fat_buffer[0], leftover, 1, fileRead);

		for (uint8_t k = 0; k<leftover; k += 2) {

			u8 a = g_fat_buffer[k + 0];
			u8 b = g_fat_buffer[k + 1];

			if (a == 0) {
				window[(head++)&(WINDOW_SIZE - 1)] = b;
				size_t r = fwrite(&b, 1, 1, fileWrite);
			}
			else {
				uint16_t idx = head - 1 - (b + ((a >> (8 - SHIFT)) << 8)) + WINDOW_SIZE;
				a &= REPEAT_MASK;
				for (uint16_t i = 0; i <= a; i++) {
					uint8_t v = window[(idx + i)&(WINDOW_SIZE - 1)];
					window[(head++)&(WINDOW_SIZE - 1)] = v;
					size_t r = fwrite(&v, 1, 1, fileWrite);
				}
			}
		}

	}

	fclose(fileWrite);
	fclose(fileRead);
}


u8 writeCache[16];

void LZLikeDecodeToFile3(const char* fileName)
{

	memset(g_fat_buffer, 0, FAT_BUF_SIZE);
	memset(window, 0, WINDOW_SIZE);
	memset(writeCache, 0, 16);

	FILE* fileRead;
	fopen_s(&fileRead, fileName, "rb");
	fseek(fileRead, 0, SEEK_END);
	long size = ftell(fileRead);
	fseek(fileRead, 0, SEEK_SET);
	FILE* fileWrite;
	fopen_s(&fileWrite, "C:\\Users\\Admin\\Desktop\\New folder\\temp.tap", "wb");


	printf("FAT_BUF_SIZE %d\n", FAT_BUF_SIZE);
	printf("size %d\n", size);
	printf("size/FAT_BUF_SIZE %d\n", size / FAT_BUF_SIZE);

	uint32_t head = 0;
	u16 amount = 0;
	for (uint32_t i = 0; i<size / FAT_BUF_SIZE; i++) {
		size_t r = fread(&g_fat_buffer[0], FAT_BUF_SIZE, 1, fileRead);
		for (uint8_t k = 0; k<FAT_BUF_SIZE; k += 2) {
			if (g_fat_buffer[k + 0] == 0) {
				window[(head++)&(WINDOW_SIZE - 1)] = g_fat_buffer[k + 1];
				writeCache[amount] = g_fat_buffer[k + 1];
				amount++;
				if (amount == 16) {
					fwrite(&writeCache[0], 16, 1, fileWrite);
					amount = 0;
				}
			}
			else {
				uint16_t idx = head - 1 - (g_fat_buffer[k + 1] + ((g_fat_buffer[k + 0] >> (8 - SHIFT)) << 8)) + WINDOW_SIZE;
				g_fat_buffer[k + 0] &= REPEAT_MASK;
				for (uint16_t i = 0; i <= g_fat_buffer[k + 0]; i++) {
					uint8_t v = window[(idx + i)&(WINDOW_SIZE - 1)];
					window[(head++)&(WINDOW_SIZE - 1)] = v;
					writeCache[amount] = v;
					amount++;
					if (amount == 16) {
						fwrite(&writeCache[0], 16, 1, fileWrite);
						amount = 0;
					}
				}
			}
		}
	}

	u16 leftover = size % FAT_BUF_SIZE;
	if (leftover) {
		size_t r = fread(&g_fat_buffer[0], leftover, 1, fileRead);
		for (uint8_t k = 0; k<leftover; k += 2) {
			if (g_fat_buffer[k + 0] == 0) {
				window[(head++)&(WINDOW_SIZE - 1)] = g_fat_buffer[k + 1];
				writeCache[amount] = g_fat_buffer[k + 1];
				amount++;
				if (amount == 16) {
					fwrite(&writeCache[0], 16, 1, fileWrite);
					amount = 0;
				}
			}
			else {
				uint16_t idx = head - 1 - (g_fat_buffer[k + 1] + ((g_fat_buffer[k + 0] >> (8 - SHIFT)) << 8)) + WINDOW_SIZE;
				g_fat_buffer[k + 0] &= REPEAT_MASK;
				for (uint16_t i = 0; i <= g_fat_buffer[k + 0]; i++) {
					uint8_t v = window[(idx + i)&(WINDOW_SIZE - 1)];
					window[(head++)&(WINDOW_SIZE - 1)] = v;
					writeCache[amount] = v;
					amount++;
					if (amount == 16) {
						fwrite(&writeCache[0], 16, 1, fileWrite);
						amount = 0;
					}
				}
			}
		}
	}

	if (amount > 0) {
		fwrite(&writeCache[0], amount, 1, fileWrite);
	}

	fclose(fileWrite);
	fclose(fileRead);
}


//uint16_t head = 0;
u8 amount = 0;  // write cache will be small (well under a byte !!  16bytes maybe , 32 tops )
//FILE* fileRead;
//FILE* fileWrite;

void decodeSection( /* FILE* fileRead, FILE* FileWrite ,*/ u16 amountToRead) {
	size_t r = fread(&g_fat_buffer[0], amountToRead, 1, fileRead);
	for (uint8_t k = 0; k<amountToRead; k += 2) {
		if (g_fat_buffer[k + 0] == 0) {
			window[(head++)&(WINDOW_SIZE - 1)] = g_fat_buffer[k + 1];
			writeCache[amount++] = g_fat_buffer[k + 1];
			if (amount == 16) {
				fwrite(&writeCache[0], 16, 1, fileWrite);
				amount = 0;
			}
		}
		else {
			uint16_t idx = head - 1 - (g_fat_buffer[k + 1] + ((g_fat_buffer[k + 0] >> (8 - SHIFT)) << 8)) + WINDOW_SIZE;
			g_fat_buffer[k + 0] &= REPEAT_MASK;
			for (uint16_t i = 0; i <= g_fat_buffer[k + 0]; i++) {
				uint8_t v = window[(idx + i)&(WINDOW_SIZE - 1)];
				window[(head++)&(WINDOW_SIZE - 1)] = v;
				writeCache[amount++] = v;
				if (amount == 16) {
					fwrite(&writeCache[0], 16, 1, fileWrite);
					amount = 0;
				}
			}
		}
	}
}


void LZLikeDecodeToFile4(const char* fileName) {

	head = 0;
	amount = 0;

	//FILE* fileRead;
	fopen_s(&fileRead, fileName, "rb");
	fseek(fileRead, 0, SEEK_END);
	long size = ftell(fileRead);
	fseek(fileRead, 0, SEEK_SET);
//	FILE* fileWrite;
	fopen_s(&fileWrite, "C:\\Users\\Admin\\Desktop\\New folder\\temp.tap", "wb");

	printf("FAT_BUF_SIZE %d\n", FAT_BUF_SIZE);
	printf("size %d\n", size);
	printf("size/FAT_BUF_SIZE %d\n", size / FAT_BUF_SIZE);

	for (uint32_t i = 0; i<size / FAT_BUF_SIZE; i++) {
		decodeSection(/*fileRead, fileWrite, */ FAT_BUF_SIZE);
	}

	u16 leftover = size % FAT_BUF_SIZE;
	if (leftover) {
		decodeSection(/*fileRead, fileWrite,  */ leftover);
	}

	if (amount > 0) {
		fwrite(&writeCache[0], amount, 1, fileWrite);
	}

	fclose(fileWrite);
	fclose(fileRead);
}



u8* LZLikeDecode(u8 *out, u8 *in, u8 *end) {
	u16 head = 0;
	while (in < end) {
		u8 a = *(in++);
		u8 b = *(in++);
 		if (a == 0) { 
			// no special repetition, cache and output disgarding 2nd byte
			window[(head++)&(WINDOW_SIZE - 1)] = b; 
			*(out++) = b;  
		}
		else {
			// index :  example 2 bits from 'a' and 8 bits from 'b'

			u16 idx = head - 1 - (b + ((a >> (8-SHIFT)) << 8)) + WINDOW_SIZE;
			a &= REPEAT_MASK; // use the (say 6) bits left over

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
	int i = 0;
	u8* pcurrentBlockStart = in;
	while (pcurrentBlockStart < end) {
		u16 byteOrOffset = *pcurrentBlockStart;
		u16 repetitionLength = 1;
	//	for (int wi = 0; wi < WINDOW_SIZE; wi++) {
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
					byteOrOffset = wi;		   // distance we looked back in bytes
					i++;

					if (len == REPEAT_LIMIT)
						break;
				}
			}
		}
		
		u8 v = (repetitionLength - 1) | ((byteOrOffset & 0xFF00) >> SHIFT);

		// * No sequence: writes [0],[byte]
		// * Found sequence: writes [no# repetitions 6bits + 2bits offset],[8bits offset]
		// WINDOW_SIZE uses 1024 in the case of 2^10bits
		*(out++) = v;
		*(out++) = byteOrOffset & 0xff;
		pcurrentBlockStart += repetitionLength;
	}

	//printf("%d\n", i);

	return out;
}
#include <windows.h>
int main(int argc, char **argv)
{
	string folder = "";

	if (argc == 2) {
		folder = argv[1];  // "C:\\Users\\Admin\\Desktop\\New folder\\taps2compress\\";

	//	printf("%s\n", argv[1]);
	}
	else {
		printf("ERROR: Missing command line pramater - please provide a path to your TAP files\n\n");
			
		printf("This tool finds and compresses all TAP files inside a folder\n");
		printf(" - Orignal TAP files will not deleted\n");
		printf(" - Compressed files will be created with a ZAP file extension\n\n");
		
		printf("Example usage:          Compresses tap files found in the... \n");
		printf("  TAPCOMPRESSION [.\\]                          tools run path\n");
		printf("  TAPCOMPRESSION [yourTaps\\]                   relative folder\n");
		printf("  TAPCOMPRESSION [c:\\data\\c64\\yourTaps\\]       absolute path\n");
		getchar();
		return 0;
	}
//	printf("WINDOW_SIZE=%d\n", WINDOW_SIZE);
	

	////LZLikeDecodeToFile((folder + "180.zap").c_str());
	////LZLikeDecodeToFile4((folder + "180.zap").c_str());
	//
	//LZLikeReset();

	//for (int i = 0; i < 1000; i++) {
	//	ReadDecodeAmount(g_fat_buffer, 128);
	//	fwrite(g_fat_buffer, 128, 1, fileWrite);
	//}
	//fclose(fileWrite);

	//fclose(fileRead);
	//return 0;


	WIN32_FIND_DATAA ffd;

	printf("%s\n" , folder.c_str());

	HANDLE  hFind = FindFirstFileA((folder + "*.tap").c_str(), &ffd);

	if (hFind == INVALID_HANDLE_VALUE) {
		printf("FindFirstFileA: INVALID_HANDLE_VALUE");
		return 0;
	}

	int filesize = 0;

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
	
	for (vector<string>::iterator it = files.begin(); it != files.end(); it++)
	{
		string fileName = *it; 
		printf("PROCESSING: %s\n", fileName.c_str());

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
			printf("Compressed from %d to %d bytes...", (int)size, (int)(eout - out));

			totalOriginal += size;
			totalCompressed += (eout - out);
			//----------
			string newfilename = fileName.substr(0, fileName.find_last_of('.')) + ".zap";

		

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
				printf("verify OK\n");
			}

			free(data);

			free(dest);
		}
	}
	printf("=========================\n");
	printf("Original Total   : %d\n", totalOriginal);
	printf("Compression Total: %d\n", totalCompressed);
	printf("saving %d bytes, %dKB, %dMB \n", totalOriginal - totalCompressed, (totalOriginal - totalCompressed)/1024, (totalOriginal - totalCompressed)/1024/1024);
	printf("=========================\n");
	
//	getchar();


}

int main2(int argc, char **argv)
{

	printf(" BITS_INDEX:%d\n", BITS_INDEX);
	printf(" BITS_REPEAT: %d\n", BITS_REPEAT);
	printf(" WINDOW_SIZE: %d\n", WINDOW_SIZE);
	printf(" SHIFT: %d\n", SHIFT);
	printf(" REPEAT_LIMIT: %d\n", REPEAT_LIMIT);
	printf(" REPEAT_MASK: %02x\n", REPEAT_MASK);

	if (argc != 3)
	{
		printf("Usage: c64convert source.tap dest.tpz\n");
		return 1;
	}
	FILE *f;
	fopen_s(&f, argv[1], "rb");
	if (f)
	{
		fseek(f, 0, SEEK_END);
		long size = ftell(f);
		printf("Loaded %s (%d bytes)\n", argv[1], (int)size);
		fseek(f, 0, SEEK_SET);
		u8 *data = (u8*)malloc(size);
		fread(data, 1, size, f);
		fclose(f);
		printf("Read done\n");
		u8 *out = (u8*)malloc(2 * size); // worst case
		u8 *eout = LZLikeEncode(out, data, data + size);
		printf("Compressed:%d/%d\n", (int)(eout - out), (int)size);
		FILE *g;
		fopen_s(&g, argv[2], "wb");
		if (g)
		{
			fwrite(out, 1, eout - out, g);
			fclose(g);
		}
		else
		{
			printf("Couldn't open '%s' for write\n", argv[2]);
			return 1;
		}
		u8 *dest = (u8*)malloc(size);
		u8 *edest = LZLikeDecode(dest, out, eout);


	//	LZLikeDecodeToFile("_180.tap", out, eout);


		if (memcmp(dest, data, size) != 0)
		{
			printf("Decompress error %d/%d\n", (int)(edest - dest), (int)size);
			return 1;
		}
	}
	else
	{
		printf("Couldn't open '%s' for read\n", argv[1]);
		return 1;
	}
	return 0;
}



//u8* LZLikeEncode(u8 *out, u8 *in, u8 *end) {
//     int inDataSize = (int)(end - in);
//     int currentBlockStart = 0;
//     while (currentBlockStart < inDataSize) {
//            int byteOrDistance = in[currentBlockStart];
//            int repetitionLength = 1;
//            for (int wi = 0; wi < WINDOW_SIZE; wi++) {
//                   u8* psearchIndex = &in[currentBlockStart];
//                   if (psearchIndex > in + wi) {
//                         int len = 0;
//                         while (*psearchIndex == *(psearchIndex - 1 - wi) && len < 64 && psearchIndex < end) {
//                                len++;
//                                psearchIndex++;
//                         }
//                         if (len > repetitionLength) {
//                                repetitionLength = len;           // found more repetition in the past
//                                byteOrDistance = wi;       // how far did we look back in bytes
//                         }
//                   }
//            }
//
//            int v = (repetitionLength - 1) | ((byteOrDistance >> 8) << 6);
//            *(out++) = v;
//            *(out++) = byteOrDistance;
//            currentBlockStart += repetitionLength;
//     }
//     return out;
//}

//
//
//
//u8* LZLikeEncode(u8 *out, u8 *in, u8 *end) {
//     int inDataSize = (int)(end - in);
//     int currentBlockStart = 0;
//     while (currentBlockStart < inDataSize) {
//            int byteOrDistance = in[currentBlockStart];
//            int repetitionLength = 1;
//            for (int wi = 0; wi < WINDOW_SIZE; wi++) {
//                   int searchIndex = currentBlockStart;
//    
//
//                   if (searchIndex - 1 - wi >= 0) {
//                         int len = 0;
//                         while (in[searchIndex] == in[searchIndex - 1 - wi] && len < 64 && searchIndex < inDataSize) {
//                                len++;
//                                searchIndex++;
//                         }
//                         if (len > repetitionLength) {
//                                repetitionLength = len;           // found more repetition in the past
//                                byteOrDistance = wi;       // how far did we look back in bytes
//                         }
//                   }
//            }
//
//            int v = (repetitionLength - 1) | ((byteOrDistance >> 8) << 6);
//            *(out++) = v;
//            *(out++) = byteOrDistance;
//            currentBlockStart += repetitionLength;
//     }
//     return out;
//}
//
//void printBits(size_t const size, void const * const ptr)
//{
//	unsigned char *b = (unsigned char*)ptr;
//	unsigned char byte;
//	int i, j;
//
//	for (i = size - 1; i >= 0; i--) {
//		for (j = 7; j >= 0; j--) {
//			byte = (b[i] >> j) & 1;
//			printf("%u", byte);
//		}
//	}
//	puts("");
//}