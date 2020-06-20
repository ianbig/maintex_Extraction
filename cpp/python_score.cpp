#define BOOST_FILESYSTEM_VERSION 3

#include <tidy.h>
#include <tidybuffio.h>
#include <stdio.h>
#include <errno.h>
#include <iostream>
#include <fstream>
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/filesystem.hpp>
#include <sstream>
#include <map>
#include "tree.h"

namespace pt = boost::property_tree;
namespace bfs = boost::filesystem;
std::map<std::string, int> stack;


struct Display calculate_score(char *golden_text, std::string gold_path, std::string content_buffer) {
    double F1_score = 0.0;
    double precision = 0.0;
    double recall = 0.0;
    std::string golden_string(golden_text);
    double lcs_string_length = (double)lcs(content_buffer, golden_string);
    struct Display data;

    
    precision = lcs_string_length / content_buffer.length();
    recall = lcs_string_length / golden_string.length();
    F1_score = 2 * precision * recall / ( precision + recall );

    data.path = gold_path;
    data.F1_score = F1_score;
    data.recall = recall;
    data.precision = precision;

    return data;
    

}


void maintexExtraction_calc(std::string gold_path, std::string html_path, struct Display *sum ) {

    std::ifstream fs;
    std::ifstream golden_text;
    char *buf, *gold_buf;

    std::cout << gold_path << std::endl;

    fs.open(html_path,std::ios::in);
    golden_text.open(gold_path, std::ios::in);

    if(!fs.is_open()) {
        std::cerr << "ERROR: input file not open" << std::endl;
        exit(EXIT_FAILURE);
    }

    if(!golden_text.is_open()) {
        std::cerr << "ERROR: golden text not open" << std::endl;
    }

    // allocate input array
    fs.seekg(0, fs.end);
    size_t length = fs.tellg();
    fs.seekg(0, fs.beg);

    buf = new char [length + 1];
    fs.read(buf, length);
    buf[length] = '\0';
    fs.close();

    //allocate golden text array
    golden_text.seekg(0, golden_text.end);
    size_t gold_length = golden_text.tellg();
    golden_text.seekg(0, golden_text.beg);

    gold_buf = new char [gold_length + 1];
    gold_buf[gold_length] = 0;
    golden_text.read(gold_buf, gold_length);
    golden_text.close();

   
    *sum = calculate_score(gold_buf, gold_path, buf);
}


int main (int argc, char **argv) {

    std::string gold_path = "/home/ianliu/develope/maintex_Extraction/CETD/BBC/gold/";
    std::string py_path = "/home/ianliu/develope/maintex_Extraction/python/output/";
    bfs::path path_origin(py_path);
    struct Display *sum = new struct Display [101];
    std::string filename;
    std::string path_gold_file;
    int index = 0;

    for(bfs::directory_iterator git = bfs::directory_iterator(path_origin); git != bfs::directory_iterator(); ++ git) {
        filename = git->path().filename().string();
        path_gold_file = gold_path + filename.substr(0, filename.find(".")) + ".txt";
        maintexExtraction_calc(path_gold_file, path_origin.string() + filename, &sum[index]);
        ++ index;
    }

    std::cout << std::setw(7) << std::setw(10) << "precision" << std::setw(10) 
    << "recall" << std::setw(10) << "F1 score" << std::endl;
    for(int i = 0; i < index ; i++ ) {
        std::cout << std::setw(7)  << std::setw(10)
        << sum[i].precision << std::setw(10) << sum[i].recall << std::setw(10) 
        << sum[i].F1_score << std::endl;
    }

    return 0;
}

