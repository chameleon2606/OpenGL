#include "Repacking.h"
#include <fstream>
#include <iostream>
#include "./include/json.hpp"
#include <filesystem>

using json = nlohmann::json;
using namespace std;

auto get_level_data(const string& data)
{
    ifstream f("../level_data.json");
    json levelData = json::parse(f);
    return levelData["area_voodooshop"][data];
}

vector<string> get_level_list()
{
    ifstream f("level_data.json");
    if (!f.is_open())
    {
        cout << "json does not exist\n";
        exit(0);
    }
    else
    {
        cout << "json read successfully\n";
    }
    json data = json::parse(f);

    vector<string> keys;
    for (auto& [key, value] : data.items()) {
        keys.push_back(key);
    }
    return keys;
}

vector<char> construct_raw_data()
{
    vector<char> rawData;
    for (auto &file : level_file_list) {
        cout << file << endl;
    }
}

void convert_wav(const string& wav_file)
{
    ifstream inputFile("../"+wav_file,ios::binary);
    if (!inputFile.is_open()) {
        cerr << "Error opening file\n";
    }
    if (inputFile.is_open()) {
        inputFile.seekg(20, ios::beg);
        unsigned char byte;
        inputFile.read(reinterpret_cast<char*>(&byte),1);
        if (byte!=0x01) {
            cerr << "not pcm\n";
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
            cerr << "Error opening new file\n";
        }

        newFile.write(startOfFile.data(), startOfFile.size());
        newFile.write(header, sizeof(header));
        newFile.write(restOfFile.data(), restOfFile.size());
    }
}

void repack()
{
    cout << "repacking ...\n";
}

void init()
{
    output_path = "../output/vincedata/";

    /*
    for (const string& area : get_level_list())
    {
        if (!area._Starts_with("area"))
        {
            continue;
        }
        current_level = area;
        ifstream hot(output_path + "textures.hot", std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc);
        level_file_list = get_level_data("textures");
    }
    */
}