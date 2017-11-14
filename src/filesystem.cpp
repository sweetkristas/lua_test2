#ifdef _MSC_VER
#include <codecvt>
#else
#include <boost/locale/encoding_utf.hpp>
#include <string>
#endif
#include <iostream>
#include <locale>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <filesystem>

#include "asserts.hpp"
#include "filesystem.hpp"

namespace sys
{
	namespace
	{
#ifndef _MSC_VER
		using boost::locale::conv::utf_to_utf;

		std::wstring utf8_to_wstring(const std::string& str)
		{
			return utf_to_utf<wchar_t>(str.c_str(), str.c_str() + str.size());
		}

		std::string wstring_to_utf8(const std::wstring& str)
		{
		    return utf_to_utf<char>(str.c_str(), str.c_str() + str.size());
		}  
#endif
	}

	using namespace std::experimental::filesystem;

	bool file_exists(const std::string& name)
	{
		path p(name);
		return exists(p) && is_regular_file(p);
	}

	std::string read_file(const std::string& name)
	{
		path p(name);
		ASSERT_LOG(exists(p), "Couldn't read file: {}", name);
		std::ifstream file(p.native(), std::ios_base::binary);
		std::stringstream ss;
		ss << file.rdbuf();
		return ss.str();
	}

	void write_file(const std::string& name, const std::string& data)
	{
		path p(name);
		ASSERT_LOG(p.is_absolute() == false, "Won't write absolute paths: {}", name);
		ASSERT_LOG(p.has_filename(), "No filename found in write_file path: {}", name);

		// Create any needed directories
		create_directories(p);

		// Write the file.
		std::ofstream file(name, std::ios_base::binary);
		file << data;
	}

	std::string get_absolute_path_to_file(const std::string& name)
	{
		path p(name);
		return absolute(p).generic_string();
	}

	std::string wstring_to_string(const std::wstring& ws)
	{
#ifdef _MSC_VER
		typedef std::codecvt_utf8<wchar_t> convert_type;
		std::wstring_convert<convert_type, wchar_t> converter;
		return converter.to_bytes(ws);
#else
		return wstring_to_utf8(ws);
#endif
	}

	void get_unique_files(const std::string& name, file_path_map& fpm)
	{
		path p(name);
		if(exists(p)) {
			ASSERT_LOG(is_directory(p) || is_other(p), "get_unique_files() not directory: {}", name);
			for(auto it = recursive_directory_iterator(p); it != recursive_directory_iterator(); ++it) {
				if(is_regular_file(it->path())) {
					std::string fn = wstring_to_string(it->path().filename().generic_wstring());
					std::string fp = wstring_to_string(it->path().generic_wstring());
					fpm[fn] = fp;
				}
			}
		} else {
			LOG_WARN("Path '{}' doesn't exist", p.generic_string());
		}
	}
}
