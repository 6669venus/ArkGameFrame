#include <map>
#include <vector>
#include <string>
#include <time.h>
#include "NFComm/NFPluginModule/NFIPluginManager.h"
#include "NFComm/NFCore/NFQueue.h"
#include "NFComm/RapidXML/rapidxml.hpp"
#include "NFComm/RapidXML/rapidxml_iterators.hpp"
#include "NFComm/RapidXML/rapidxml_print.hpp"
#include "NFComm/RapidXML/rapidxml_utils.hpp"
#include "NFComm/NFPluginModule/NFPlatform.h"

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>

#if NF_PLATFORM == NF_PLATFORM_WIN
#include <io.h>
#include <windows.h>
#include <conio.h>
#else
#include <iconv.h>
#include <unistd.h>
#include <cstdio>
#include <dirent.h>
#include <sys/stat.h>
#endif

bool GetPluginNameList(std::string& strPlugin, std::vector<std::string>& pluginList, std::string& configPath)
{
	rapidxml::file<> fdoc(strPlugin.c_str());
	rapidxml::xml_document<>  doc;
	doc.parse<0>(fdoc.data());

	rapidxml::xml_node<>* pRoot = doc.first_node();
	for (rapidxml::xml_node<>* pPluginNode = pRoot->first_node("Plugin"); pPluginNode; pPluginNode = pPluginNode->next_sibling("Plugin"))
	{
		const char* strPluginName = pPluginNode->first_attribute("Name")->value();
		const char* strMain = pPluginNode->first_attribute("Main")->value();
		pluginList.push_back(std::string(strPluginName));
	}

	rapidxml::xml_node<>* pPluginAppNode = pRoot->first_node("APPID");
	if (!pPluginAppNode)
	{
		//NFASSERT(0, "There are no App ID", __FILE__, __FUNCTION__);
		return false;
	}

	const char* strAppID = pPluginAppNode->first_attribute("Name")->value();
	if (!strAppID)
	{
		//NFASSERT(0, "There are no App ID", __FILE__, __FUNCTION__);
		return false;
	}

	rapidxml::xml_node<>* pPluginConfigPathNode = pRoot->first_node("ConfigPath");
	if (!pPluginConfigPathNode)
	{
		//NFASSERT(0, "There are no ConfigPath", __FILE__, __FUNCTION__);
		return false;
	}

	if (NULL == pPluginConfigPathNode->first_attribute("Name"))
	{
		//NFASSERT(0, "There are no ConfigPath.Name", __FILE__, __FUNCTION__);
		return false;
	}

	configPath = pPluginConfigPathNode->first_attribute("Name")->value();

	return true;
}

int CopyFile(std::string& SourceFile, std::string& NewFile)
{
	ifstream in;
	ofstream out;
	in.open(SourceFile.c_str(), ios::binary);//打开源文件
	if (in.fail())//打开源文件失败
	{
		cout << "Error 1: Fail to open the source file." << endl;
		in.close();
		out.close();
		return 0;
	}
	out.open(NewFile.c_str(), ios::binary);//创建目标文件 
	if (out.fail())//创建文件失败
	{
		cout << "Error 2: Fail to create the new file." << endl;
		out.close();
		in.close();
		return 0;
	}
	else//复制文件
	{
		out << in.rdbuf();
		out.close();
		in.close();
		return 1;
	}
}

std::vector<std::string> GetFileListInFolder(std::string folderPath, int depth)
{
	std::vector<std::string> result;
#if NF_PLATFORM == NF_PLATFORM_WIN
	_finddata_t FileInfo;
	std::string strfind = folderPath + "\\*";
	long long Handle = _findfirst(strfind.c_str(), &FileInfo);


	if (Handle == -1L)
	{
		std::cerr << "can not match the folder path" << std::endl;
		exit(-1);
	}
	do {
		//判断是否有子目录
		if (FileInfo.attrib & _A_SUBDIR)
		{
			//这个语句很重要
			if ((strcmp(FileInfo.name, ".") != 0) && (strcmp(FileInfo.name, "..") != 0))
			{
				std::string newPath = folderPath + "\\" + FileInfo.name;
				//dfsFolder(newPath, depth);
				auto newResult = GetFileListInFolder(newPath, depth);
				result.insert(result.begin(), newResult.begin(), newResult.end());
			}
		}
		else
		{

			std::string filename = (folderPath + "\\" + FileInfo.name);
			result.push_back(filename);
		}
	} while (_findnext(Handle, &FileInfo) == 0);


	_findclose(Handle);
#else
	DIR *dp;
	struct dirent *entry;
	struct stat statbuf;
	char absolutepath[512];

	if ((dp = opendir(folderPath.c_str())) == NULL) {
		fprintf(stderr, "Can`t open directory %s\n", folderPath.c_str());
		return result;
	}

	while ((entry = readdir(dp)) != NULL) {
		std::string strEntry = folderPath + entry->d_name;
		lstat(strEntry.c_str(), &statbuf);
		if (S_ISDIR(statbuf.st_mode)) {
			if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
			{
				continue;
			}
			std::string childpath = folderPath + "/" + entry->d_name;
			auto newResult = GetFileListInFolder(childpath, depth);
			result.insert(result.begin(), newResult.begin(), newResult.end());
		}
		else
		{
			sprintf(absolutepath, "%s/%s", folderPath.c_str(), entry->d_name);
			result.push_back(absolutepath);
		}
	}
	closedir(dp);
	sort(result.begin(), result.end());//排序
#endif
	return result;
}

void StringReplace(std::string &strBig, const std::string &strsrc, const std::string &strdst)
{
	std::string::size_type pos = 0;
	std::string::size_type srclen = strsrc.size();
	std::string::size_type dstlen = strdst.size();

	while ((pos = strBig.find(strsrc, pos)) != std::string::npos)
	{
		strBig.replace(pos, srclen, strdst);
		pos += dstlen;
	}
}


void printResult(int result, std::string& strName)
{
	if (result == 1)
	{
		printf("Copy file: %s success!\n", strName.c_str());
	}
	else
	{
		printf("Copy file: %s failed!\n", strName.c_str());
	}
}

int main()
{
	std::vector<std::string> fileList;
	std::vector<std::string> errorFileList;
#if NF_PLATFORM == NF_PLATFORM_WIN
	printf("NFAutoCopyDll is Running in Windows...\n");
#else
	printf("NFAutoCopyDll is Running in Linux...\n");
#endif
#ifdef NF_DEBUG_MODE
	fileList = GetFileListInFolder("../../Debug/", 10);
	printf("Debug Mode, And fileList size == %d\n", fileList.size());
#else
	fileList = GetFileListInFolder("../../Release/", 10);
	printf("Release Mode, And fileList size == %d\n", fileList.size());
#endif

	for (auto fileName : fileList)
	{
		if (fileName.find("Plugin.xml") != std::string::npos)
		{
			StringReplace(fileName, "\\", "/");
			StringReplace(fileName, "//", "/");
			printf("Reading xml file: %s\n", fileName.c_str());

			std::vector<std::string> pluginList;
			std::string configPath = "";
			GetPluginNameList(fileName, pluginList, configPath);
			if (pluginList.size() > 0 && configPath != "")
			{
#if NF_PLATFORM == NF_PLATFORM_WIN
				pluginList.push_back("libprotobuf");
				pluginList.push_back("NFMessageDefine");
#else
				pluginList.push_back("NFMessageDefine");
#endif
				pluginList.push_back("NFPluginLoader");
				configPath = "../" + configPath;
				configPath = fileName.substr(0, fileName.find_last_of("/")) + "/" + configPath;

				for (std::string name : pluginList)
				{
					int result = 0;
					if (name == "NFPluginLoader")
					{
						int result = 0;
#if NF_PLATFORM == NF_PLATFORM_WIN

#ifdef NF_DEBUG_MODE
						std::string src = configPath + "Comm/Debug/" + name;
						std::string des = fileName.substr(0, fileName.find_last_of("/")) + "/" + name;
						auto strSrc = src + "_d.exe";
						auto strDes = des + "_d.exe";
						auto strSrcPDB = src + "_d.pdb";
						auto strDesPDB = des + "_d.pdb";

						result = CopyFile(strSrc, strDes);
						if (result != 1)
						{
							errorFileList.push_back(strSrc);
						}
						printResult(result, strSrc);

						result = CopyFile(strSrcPDB, strDesPDB);
						if (result != 1)
						{
							errorFileList.push_back(strSrcPDB);
						}
						printResult(result, strSrcPDB);
#else
						std::string src = configPath + "Comm/Release/" + name;
						std::string des = fileName.substr(0, fileName.find_last_of("/")) + "/" + name;
						auto strSrc = src + ".exe";
						auto strDes = des + ".exe";
						int result = 0;
						result = CopyFile(strSrc, strDes);
						if (result != 1)
						{
							errorFileList.push_back(strSrc);
						}
						printResult(result, strSrc);
#endif

#else
						std::string src = configPath + "Comm/" + name;
						std::string des = fileName.substr(0, fileName.find_last_of("/")) + "/" + name;
						auto strSrc = src + "_d";
						auto strDes = des + "_d";
						result = CopyFile(strSrc, strDes);
						if (result != 1)
						{
							errorFileList.push_back(strSrc);
						}
						printResult(result, strSrc);
#endif

					}
					else
					{
#if NF_PLATFORM == NF_PLATFORM_WIN

#ifdef NF_DEBUG_MODE
						std::string src = configPath + "Comm/Debug/" + name;
						std::string des = fileName.substr(0, fileName.find_last_of("/")) + "/" + name;
						auto strSrc = src + "_d.dll";
						auto strDes = des + "_d.dll";
						auto strSrcPDB = src + "_d.pdb";
						auto strDesPDB = des + "_d.pdb";
						result = CopyFile(strSrc, strDes);
						if (result != 1)
						{
							errorFileList.push_back(strSrc);
						}
						printResult(result, strSrc);

						result = CopyFile(strSrcPDB, strDesPDB);
						if (result != 1)
						{
							errorFileList.push_back(strSrcPDB);
						}
						printResult(result, strSrcPDB);
#else
						std::string src = configPath + "Comm/Release/" + name;
						std::string des = fileName.substr(0, fileName.find_last_of("/")) + "/" + name;
						auto strSrc = src + ".dll";
						auto strDes = des + ".dll";
						result = CopyFile(strSrc, strDes);
						if (result != 1)
						{
							errorFileList.push_back(strSrc);
						}
						printResult(result, strSrc);
#endif

#else
						std::string src = configPath + "Comm/lib" + name;
						std::string des = fileName.substr(0, fileName.find_last_of("/")) + "/" + name;
						auto strSrc = src + "_d.so";
						auto strDes = des + "_d.so";
						result = CopyFile(strSrc, strDes);
						if (result != 1)
						{
							errorFileList.push_back(strSrc);
						}
						printResult(result, strSrc);
#endif
					}
				}
			}
		}
	}
	if (errorFileList.size() <= 0)
	{
		printf("File Copy Done with NO ERROR!\n");
	}
	else
	{
		printf("File Copy Done with %d errors as follow:\n", (int)errorFileList.size());
		for (auto errFile : errorFileList)
		{
			printf("\t%s\n", errFile.c_str());
		}
	}
#if NF_PLATFORM == NF_PLATFORM_WIN
	printf("Press any Key to Exit!");
	getch();
#else

#endif
	return 0;
}