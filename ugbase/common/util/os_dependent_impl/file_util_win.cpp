// created by Sebastian Reiter
// s.b.reiter@googlemail.com
// 14.09.2011 (m,d,y)
#define _WIN32_IE 0x0501 //0x0400 //required so that SHGetSpecialFolderPath is exposed by shlobj.h
#include <windows.h>
#include "shlobj.h"
#include "common/util/file_util.h"
#include "common/error.h"
#include "common/log.h"
#include <direct.h>

using namespace std;

namespace ug{

bool DirectoryExists(const char* dirname)
{
	WIN32_FIND_DATA findData;

//	check if directory exists
	if(FindFirstFile(dirname, &findData) ==
		INVALID_HANDLE_VALUE)
	{
	//	no
		return false;
	}
	return true;
}
	
//	This method returns a list of all directories in a directory
bool GetDirectoriesInDirectory(std::vector<std::string>& dirsOut, const char* dir)
{
	dirsOut.clear();

	string expr = dir;
	expr.append("\\*");

	WIN32_FIND_DATA	findData;
	HANDLE hFind;
	
	hFind = FindFirstFile(expr.c_str(), &findData);
	if(hFind == INVALID_HANDLE_VALUE){
		return true;
	}
	
	do
	{
		if(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
			dirsOut.push_back(string(findData.cFileName));
		}
	}
	while(FindNextFile(hFind, &findData) != 0);
	
	FindClose(hFind);

	return true;
}

//	This method returns a list of all files in a directory
bool GetFilesInDirectory(std::vector<std::string>& filesOut, const char* dir)
{

	filesOut.clear();

	string expr = dir;
	expr.append("\\*");

	WIN32_FIND_DATA	findData;
	HANDLE hFind;
	
	hFind = FindFirstFile(expr.c_str(), &findData);
	if(hFind == INVALID_HANDLE_VALUE){
		return true;
	}
	
	do
	{
		if(!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			filesOut.push_back(string(findData.cFileName));
		}
	}
	while(FindNextFile(hFind, &findData) != 0);
	
	FindClose(hFind);

	return true;
}

bool CreateDirectoryTMP(const char *directory)
{
	return _mkdir(directory) == 0;
}

bool CreateDirectory(const char *directory)
{
	return _mkdir(directory) == 0;
}

bool CreateDirectory(const char *directory, int mode)
{
	return _mkdir(directory) == 0;
}

bool CreateDirectory(std::string directory)
{
	return CreateDirectory(directory.c_str());
}

std::string GetTmpPath()
{
	static string userDataPath("");

	if(userDataPath.empty())
	{
		char tFolderPath[MAX_PATH];
		ZeroMemory(tFolderPath, MAX_PATH);

	//	init the data path
		SHGetSpecialFolderPath(0, tFolderPath, CSIDL_APPDATA, 1);

		userDataPath = tFolderPath;
	}

	return userDataPath.c_str();
}

void ChangeDirectory(std::string dir)
{
	if(chdir(dir.c_str()) != 0){
		UG_THROW("ChangeDirectory failed.");
	}
}

}// end of namespace
