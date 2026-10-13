#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

#ifndef WORKDIR
#define WORKDIR "D:/VestEngine/"
#endif
		
class File
{
	static const std::string projectDir;
	static const std::string resourcesDir;

public:
	static const std::string& getProjectDir() { return projectDir; }
	static const std::string& getResourcesDir() { return resourcesDir; }

	static bool readFile(const std::string& inPath, std::vector<char>& outContent)
	{
		std::ifstream file(inPath, std::ios::ate | std::ios::binary);

		if (!file.is_open()) 
		{
			std::cout << "ERROR: Could not read file " << inPath << std::endl;
			return false;
		}

		size_t fileSize = (size_t)file.tellg();
		outContent.resize(fileSize);

		file.seekg(0);
		file.read(outContent.data(), fileSize);
		file.close();

		return true;
	}

	static bool readFile(const std::string& inPath, std::string& outContent)
	{
		std::string line;
		std::ifstream in(inPath);
		if (in.fail())
		{
			std::cout << "ERROR: Could not read file " << inPath << std::endl;
			return false;
		}

		outContent.clear();
		while (std::getline(in, line))
		{
			outContent += line + "\n";
		}
		in.close();

		return true;
	}
};