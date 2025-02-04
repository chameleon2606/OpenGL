#pragma once
#include <string>
#include <vector>
#include <shlobj.h>

#pragma once

static std::string output_path;
void init();
static auto get_level_data(const std::string& data);
static std::vector<char> construct_raw_data();
static void convert_wav(const std::string& wav_file);
static std::vector<std::string> level_file_list;
static std::string current_level;
static std::vector<char> raw_data_bytes;
void repack();
std::string open_file();