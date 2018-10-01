#pragma once

#ifndef FileHandlerHeader
#define FileHandlerHeader

#include <iostream>

extern long long int records[10];
extern bool soundEnabled;

// return true if file not exist and created, otherwise false if file exist
// if error happened app will be closed.
bool CreateFile(const char* fileName) // C11 syntax for read file
{
	FILE *filePtr;
	errno_t err; // If successful, returns zero and a pointer to the new file stream is written to *filePtr.
				 // On error, returns a non - zero error code and writes the null pointer to *filePtr. (unless filePtr is a null pointer itself)
	if ((err = fopen_s(&filePtr, fileName, "r")) != 0)
	{
		if ((err = fopen_s(&filePtr, fileName, "w")) != 0)
			throw std::runtime_error{ "File couldn't create" };
		else // file created and exist now
		{
			fclose(filePtr); // we just close it
			return true;
		}
	}

	// file exist and we opened it
	fclose(filePtr); // we just close it
	return false;
}

// Load settings if exist, otherwise create them for first time 
void LoadSettings()
{
	FILE *filePtr;
	if (CreateFile("settings.txt") == true) // if true, it means file created for first time, so we initialize our settings
	{
		fopen_s(&filePtr, "settings.txt", "wb"); // We create and overwrite default settings
		fwrite(&soundEnabled, sizeof(bool), 1, filePtr);
		fwrite(&records, sizeof(long long int), 10, filePtr);
		fclose(filePtr);
	}
	else // file exist, so settings are available and we load them 
	{
		fopen_s(&filePtr, "settings.txt", "rb");
		fread(&soundEnabled, sizeof(bool), 1, filePtr);
		fread(&records, sizeof(long long int), 10, filePtr);
		fclose(filePtr);
	}
}

// Update settings
void UpdateSettings()
{
	FILE *filePtr;
	if (CreateFile("settings.txt") == false) // if false, it means file exist
	{
		fopen_s(&filePtr, "settings.txt", "wb"); // We create and overwrite settings
		fwrite(&soundEnabled, sizeof(bool), 1, filePtr);
		fwrite(&records, sizeof(long long int), 10, filePtr);
		fclose(filePtr);
	}
	else // file not exist, so error happened
		throw std::runtime_error{ "Can't find settings" };
}

#endif // !FileHandlerHeader

