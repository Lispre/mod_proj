#include <iostream>
#include <string>
#include <vector>

#include <ShlObj.h>

#include "tinyxml2.h"

#pragma comment(lib, "Shell32.lib")

using namespace tinyxml2;

void traverse_xml(XMLElement* root)
{
	if (root == nullptr)
	{
		return;
	}
	
	std::string rootName = root->Name();
	XMLElement* first = root->FirstChildElement();
	if (rootName == "CharacterSet")
	{
		root->SetText("MultiByte");
	}

	if (rootName == "PropertyGroup" && 
		(root->FirstChildElement("LinkIncremental") != nullptr) &&
		(root->FirstChildElement("OutDir") == nullptr))
	{
		XMLElement* out_dir = root->GetDocument()->NewElement("OutDir");
		XMLText* out_dir_text = root->GetDocument()->NewText(R"($(SolutionDir)build\$(Platform)\$(Configuration)\)");
		out_dir->LinkEndChild(out_dir_text);
		root->InsertEndChild(out_dir);

		out_dir = root->GetDocument()->NewElement("IntDir");
		out_dir_text = root->GetDocument()->NewText(R"($(SolutionDir)build\$(Platform)\$(Configuration)\)");
		out_dir->LinkEndChild(out_dir_text);
		root->InsertEndChild(out_dir);
	}

	if (rootName == "ItemDefinitionGroup")
	{
		XMLElement* build_log = root->FirstChildElement("BuildLog");
		if (build_log == nullptr)
		{
			build_log = root->GetDocument()->NewElement("BuildLog");
			root->InsertEndChild(build_log);
			XMLElement* path = root->GetDocument()->NewElement("Path");
			XMLText* path_text = root->GetDocument()->NewText(R"($(SolutionDir)build\$(Platform)\$(Configuration)\BuildLog\$(MSBuildProjectName).log)");
			path->LinkEndChild(path_text);
			build_log->InsertEndChild(path);
		}
		XMLElement* clcompile = root->FirstChildElement("ClCompile");
		if (clcompile->FirstChildElement("SuppressStartupBanner") == nullptr)
		{
			XMLElement* new_node = clcompile->GetDocument()->NewElement("SuppressStartupBanner");
			XMLText* new_text = clcompile->GetDocument()->NewText("false");
			new_node->LinkEndChild(new_text);
			clcompile->InsertEndChild(new_node);
		}
	}

	if (rootName == "Link" && (root->FirstChildElement("AdditionalLibraryDirectories") == nullptr))
	{
		XMLElement* new_node = root->GetDocument()->NewElement("AdditionalLibraryDirectories");
		XMLText* new_text = root->GetDocument()->NewText(R"($(ProjectDir)lib\$(Platform)\$(Configuration)\)");
		new_node->LinkEndChild(new_text);
		root->InsertEndChild(new_node);
	}
	
	XMLElement* child = root->FirstChildElement();
	if (child != nullptr)
	{
		traverse_xml(child);
	}
	XMLElement* next_sibling = root->NextSiblingElement();
	if (next_sibling != nullptr)
	{
		traverse_xml(next_sibling);
	}
	else
	{
		return;
	}
}


void SplitString(const std::string& s, std::vector<std::string>& v, const std::string& c)
{
	std::string::size_type pos1, pos2;
	pos2 = s.find(c);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		v.push_back(s.substr(pos1, pos2 - pos1));

		pos1 = pos2 + c.size();
		pos2 = s.find(c, pos1);
	}

	if (pos1 != s.length())
	{
		v.push_back(s.substr(pos1));
	}
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "mod_proj *.vcxproj" << std::endl;
		return -1;
	}

	std::vector<std::string> all_paths{};
	all_paths.push_back(R"(lib\x64\Debug)");
	all_paths.push_back(R"(lib\x64\Release)");
	all_paths.push_back(R"(lib\x32\Debug)");
	all_paths.push_back(R"(lib\x32\Release)");

	for (auto& path : all_paths)
	{
		std::vector<std::string> folders;

		SplitString(path, folders, "\\");
		std::string temp{"."};
		for (auto& v : folders)
		{
			CreateDirectory(temp.append("\\").append(v).c_str(), nullptr);
		}
	}

	tinyxml2::XMLDocument doc;
	doc.LoadFile(argv[1]);
	XMLElement* root = doc.RootElement();
	traverse_xml(root);
	doc.SaveFile(argv[1]);

	return 0;
}