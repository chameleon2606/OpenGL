#pragma once
#include <string>
#include <vector>

#pragma once

static constexpr bool IS_DEBUG = true;
static std::string currentLevel;
static std::string* currentFile;
static std::string outputPath = "F:/DotNetProjects/OpenGL/output/";
static std::vector<std::string> level_file_list;
static std::vector<char> raw_data_bytes;
static std::string TEXTURE_PATH = "C:/Users/leong/PycharmProjects/texture_repacker/textures/";
//static std::string TEXTURE_PATH = "/Users/l.gerstetter/PycharmProjects/voodoo_vince_texture_repacker/textures/";
static constexpr int DDS_HEADER_SIZE = 128;
static constexpr int HOT_HEADER_SIZE = 36;
static constexpr int METADATA_SIZE = 32;
static std::vector<char> filename_table;
static std::vector<int> data_index;
static std::vector<char> metadata;
static int file_headers_size;
static int raw_data_size;
static const std::string JSON_FILE_PATH = "F:/DotNetProjects/OpenGL/level_data.json";
static char hot_file_path[128];
static std::string repacking_mode;
static float level_progress, files_progress;
static int e;

void repack();
void update();