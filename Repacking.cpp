#include "Repacking.h"
#include <fstream>
#include <iostream>
#include "./include/json.hpp"
#include <filesystem>
#include <imgui.h>

using json = nlohmann::json;
using namespace std;

std::vector<uint8_t> int_to_bytes(uint32_t value) {
    std::vector<uint8_t> bytes(4);
    for (int i = 0; i < 4; i++) {
        bytes[i] = static_cast<uint8_t>(value >> (8 * i));
    }
    return bytes;
}

void conver_wav(const string& wav_file) {
    ifstream inputFile("../"+wav_file,ios::binary);
    if (!inputFile.is_open()) {
        cerr << "Error opening file" << endl;
        return;
    }
    inputFile.seekg(20, ios::beg);
    unsigned char byte;
    inputFile.read(reinterpret_cast<char*>(&byte),1);
    if (byte!=0x01) {
        cerr << "not pcm" << endl;
        return;
    }

    inputFile.seekg(64, ios::beg);
    vector<char> buffer(4);
    inputFile.read(buffer.data(), buffer.size());
    string bufferString;
    for (const char byte1 : buffer) {
        bufferString += byte1;
    }
    if(bufferString == "data") {
        return;
    }

    inputFile.seekg(0, ios::beg);
    vector<char> startOfFile(36);
    inputFile.read(startOfFile.data(), startOfFile.size());

    inputFile.seekg(0, ios::end);
    const int fileSize = inputFile.tellg();

    inputFile.seekg(36, ios::beg);
    vector<char> restOfFile(fileSize-36);
    inputFile.read(restOfFile.data(), restOfFile.size());
    constexpr char header[28] = {'P', 'A', 'A', 'D', 0x14, 0x00, 0x00, 0x00};

    fstream newFile("../bin2.txt", ios::in | ios::out | ios::binary | ios::trunc);
    if (!newFile) {
        cerr << "Error opening new file" << endl;
        return;
    }

    newFile.write(startOfFile.data(), startOfFile.size());
    newFile.write(header, sizeof(header));
    newFile.write(restOfFile.data(), restOfFile.size());
}

auto getLevelData(const string& data) {
    ifstream f(JSON_FILE_PATH);
    json jsonData = json::parse(f);
    return jsonData[currentLevel][data];
}

vector<string> getLevelList() {
    ifstream f(JSON_FILE_PATH);
    json jsonData = json::parse(f);

    vector<string> keys;
    for (auto& [key, value] : jsonData.items()) {
        keys.push_back(key);
    }
    return keys;
}

vector<char> constructRawData() {
    vector<char> raw_data;
    data_index.clear();
    for (string &file : level_file_list) {
        currentFile = &file;
        string src_path;
        if (file.starts_with("lightmap")) {
            src_path = TEXTURE_PATH+"/lightmaps/";
            src_path += currentLevel+"/";
        }
        else {
            src_path = TEXTURE_PATH;
        }
        ifstream src_file(src_path+file);

        if (!src_file.is_open()) {
            cerr << file << " does not exist!\n";
            exit(1);
        }
        data_index.push_back(raw_data.size());
        src_file.seekg(DDS_HEADER_SIZE, ios::end);  // seek to the header size to calculate the file size without the header
        const int fileSize = src_file.tellg();
        vector<char> buffer(fileSize);
        src_file.seekg(DDS_HEADER_SIZE, ios::beg);
        src_file.read(buffer.data(), fileSize);
        raw_data.insert(raw_data.end(), buffer.begin(), buffer.end());
        int offset = raw_data.size();
        raw_data.insert(raw_data.end(), (ceil(offset / 128.0f) * 128) - offset, 0x00);
    }
    return raw_data;
}

vector<char> construct_filenames() {
    vector<char> charlist;
    for (const auto& file : level_file_list) {

        const char* filename_bytes = file.c_str();
        const int byte_length = strlen(filename_bytes);
        int filename_byte_remainder = -byte_length - 4 * static_cast<int>(floor(static_cast<double>(-byte_length) / 4));

        if (filename_byte_remainder == 0) {
            filename_byte_remainder = 4;
        }

        charlist.insert(charlist.end(), filename_bytes, filename_bytes+byte_length);

        charlist.insert(charlist.end(), filename_byte_remainder, 0x00);
    }

    const int offset = HOT_HEADER_SIZE + ((METADATA_SIZE * level_file_list.size()) - 8) + charlist.size();
    charlist.insert(charlist.end(), (ceil(offset / 128.0f) * 128) - offset, 0x00);

    return charlist;
}

vector<char> construct_file_metadata() {
    const auto mystery_number_array = getLevelData("mystery_dds_numbers");
    vector<char> file_info_table;
    vector<char> dds_header_size_in_bytes(4);
    vector<uint8_t> dds_bytes = int_to_bytes(DDS_HEADER_SIZE);
    int headers_offset = HOT_HEADER_SIZE + ((METADATA_SIZE * level_file_list.size()) - 8) + filename_table.size();
    int data_offset = headers_offset + (DDS_HEADER_SIZE * level_file_list.size());
    int i = 0;
    int level_list_number = level_file_list.size();

    for (const string& file : level_file_list) {
        string src_path;
        if (file.starts_with("lightmap")) {
            src_path = TEXTURE_PATH+"/lightmaps/";
            src_path += currentLevel+"/";
        }
        else {
            src_path = TEXTURE_PATH;
        }
        file_info_table.insert(file_info_table.end(), dds_bytes.begin(), dds_bytes.end());  // header size
        vector<uint8_t> this_header_offset = int_to_bytes(headers_offset + (DDS_HEADER_SIZE * i));
        file_info_table.insert(file_info_table.end(), this_header_offset.begin(), this_header_offset.end());  // header offset
        vector<uint8_t> file_size = int_to_bytes(filesystem::file_size(src_path + file));
        file_info_table.insert(file_info_table.end(), file_size.begin(), file_size.end());  // file size
        file_info_table.insert(file_info_table.end(), 4, 0x00);  // blank
        vector<uint8_t> file_offset = int_to_bytes(data_offset + data_index[i]);  // file offset
        file_info_table.insert(file_info_table.end(), file_offset.begin(), file_offset.end());  // file offset
        file_info_table.insert(file_info_table.end(), 4, 0x00);  // blank
        if (i < level_list_number - 1) {
            vector<uint8_t> mystery_number = int_to_bytes(mystery_number_array[i]);
            file_info_table.insert(file_info_table.end(), mystery_number.begin(), mystery_number.end());  // mystery number
            file_info_table.insert(file_info_table.end(), 4, 0x00);  // blank
            i++;
        }
    }
    return file_info_table;
}

vector<char> construct_file_headers() {
    vector<char> file_headers;
    int i = 0;
    for (const string& file : level_file_list) {
        string src_path;
        if (file.starts_with("lightmap")) {
            src_path = TEXTURE_PATH+"/lightmaps/";
            src_path += currentLevel+"/";
        }
        else {
            src_path = TEXTURE_PATH;
        }
        ifstream src_file(src_path+file);
        vector<char> buffer(DDS_HEADER_SIZE);
        src_file.read(buffer.data(), DDS_HEADER_SIZE);
        file_headers.insert(file_headers.end(), buffer.begin(), buffer.end());
        i++;
    }
    int offset = file_headers.size();
    file_headers.insert(file_headers.end(), (ceil(offset / 128.0f) * 128) - offset, 0x00);
    return file_headers;
}

vector<char> construct_header() {
    vector<char> header;
    char signature[8] = {'H','O','T',' ', 0x01, 0x00, 0x00, 0x00};
    header.insert(header.end(), signature, signature+8);
    vector<uint8_t> value = int_to_bytes(HOT_HEADER_SIZE + metadata.size() + filename_table.size());
    header.insert(header.end(), value.begin(), value.end());
    vector<uint8_t> value2 = int_to_bytes(DDS_HEADER_SIZE + ((METADATA_SIZE * level_file_list.size()) - 8)
        + filename_table.size() + (DDS_HEADER_SIZE * level_file_list.size()));  // data offset
    header.insert(header.end(), value2.begin(), value2.end());
    vector<uint8_t> value3 = int_to_bytes(HOT_HEADER_SIZE + metadata.size() + filename_table.size() + file_headers_size + raw_data_size);  // total size
    header.insert(header.end(), value3.begin(), value3.end());
    vector<uint8_t> value4 = int_to_bytes(HOT_HEADER_SIZE + metadata.size());  // writes the offset of the filename table
    header.insert(header.end(), value4.begin(), value4.end());
    vector<uint8_t> value5 = int_to_bytes(level_file_list.size());  // writes the amount of textures or sounds
    header.insert(header.end(), value5.begin(), value5.end());

    return header;
}

void validate_directory() {
    ifstream f(JSON_FILE_PATH);
    if (!f.is_open())
    {
        cout << "json file does not exist\n";
    }
    json jsonData = json::parse(f);
    if (!filesystem::exists(outputPath + "vincedata/")) {
        cerr << "Invalid folder\n";
        exit(69);
    }

    outputPath =+ "vincedata/";

    if (IS_DEBUG) {
        for (const string& area : getLevelList()) {
            if (!filesystem::exists(jsonData[area]["path"])) {
                filesystem::create_directories(outputPath + string(jsonData[area]["path"]));
            }
        }
    }
}

void repack()
{
    cout << "Repacking\n";
    validate_directory();
    
    for (const string& area : getLevelList()) {
        cout << area << endl;
        level_file_list.clear();
        if (!area.starts_with("area")) {
            continue;
        }
        currentLevel = area;

        if (!filesystem::exists(outputPath + string(getLevelData("path")))) {
            cerr << "Invalid folder\n";
            exit(420);
        }
        fstream hot(outputPath + string(getLevelData("path")) + "textures.hot", ios::in | ios::out | ios::binary | ios::trunc);

        level_file_list = getLevelData("textures");

        raw_data_bytes = constructRawData();
        raw_data_size = raw_data_bytes.size();
        filename_table = construct_filenames();

        vector<char> header_placeholder;
        header_placeholder.insert(header_placeholder.end(), HOT_HEADER_SIZE, 0x00);
        hot.write(header_placeholder.data(), header_placeholder.size());

        metadata = construct_file_metadata();
        hot.write(metadata.data(), metadata.size());

        hot.write(filename_table.data(), filename_table.size());

        vector<char> file_headers = construct_file_headers();
        file_headers_size = file_headers.size();
        hot.write(file_headers.data(), file_headers_size);

        hot.write(raw_data_bytes.data(), raw_data_bytes.size());

        vector<char> header_bytes = construct_header();
        hot.seekg(0, ios::beg);
        hot.write(header_bytes.data(), header_bytes.size());
    }
    
}

void update()
{
    ImGui::Text("Chameleon's texture and sound repacker");
    ImGui::Dummy(ImVec2(0, 20));
    
    if (ImGui::Button("choose game folder"))
    {
        cout << "choosing game folder..\n";
    }
    ImGui::SameLine();
    ImGui::InputText("Path", hot_file_path, IM_ARRAYSIZE(hot_file_path));
    ImGui::Text(hot_file_path);

    ImGui::Dummy(ImVec2(0,30));
    
    if (ImGui::RadioButton("textures", &e, 0))
    {
        repacking_mode = "textures";
    }
    if (ImGui::RadioButton("sounds", &e, 1))
    {
        repacking_mode = "sounds";
    }
    
    ImGui::Dummy(ImVec2(0,30));
    
    if (ImGui::Button("Repack"))
    {
        repack();
    }

    ImGui::Dummy(ImVec2(0,30));
    
    ImGui::ProgressBar(level_progress, ImVec2(0, 0));
    ImGui::SameLine();
    ImGui::Text("level progress");

    ImGui::ProgressBar(files_progress, ImVec2(0, 0));
    ImGui::SameLine();
    ImGui::Text("Files progress");
}