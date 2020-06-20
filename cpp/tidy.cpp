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


void maintexExtraction(std::string gold_path, std::string html_path, struct Display *sum ) {
    std::ifstream fs;
    std::ifstream golden_text;
    char *buf, *gold_buf;


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


    // start using tidy
    TidyBuffer output = {0};
    TidyBuffer errbuf = {0};
    int rc = -1;
    bool ok;
    ctmbstr encoding = "utf8";

    TidyDoc doc = tidyCreate(); // Initialize "document"
    ok = tidyOptSetBool(doc, TidyXmlOut, yes); // Convert to XHTML
    tidySetCharEncoding(doc,encoding);
    
    if(ok)
        rc = tidySetErrorBuffer(doc, &errbuf); // Capture diagnostics
    if(rc >= 0 ) {
        rc = tidyParseString(doc, buf); // Parse the input
    }
    if ( rc >= 0 )
        rc = tidyCleanAndRepair( doc ); // Tidy it up!
    if(rc >= 0)
        rc = tidyRunDiagnostics(doc);
    if( rc > 1)
        rc = ( tidyOptSetBool(doc, TidyForceOutput, yes) ? rc : -1 ); // If error, force output.
    if ( rc >= 0 )
        rc = tidySaveBuffer( doc, &output );          // Pretty Print
    
   
    // parse DOM and compute textdensity (Ci / Ti)
    boost::property_tree::ptree tree;
    std::stringstream ss;
    ss << output.bp;

    try {
        boost::property_tree::xml_parser::read_xml(ss, tree);
    }

    catch(const boost::property_tree::xml_parser::xml_parser_error& ex) {
        std::cerr << ex.message() << std::endl;
    }

    // build tree and calculate score
    tree_node *root;
    //std::cout << "========Create Tree=========" << std::endl;
    root = create_dom_tree(tree, "root"); // root means on top of tree = first call of function
    DOM_tree droot(root);
    //std::cout << "Tree Built Complete" << std::endl;
    //std::cout << "========Extract Content=========" << std::endl;
    droot.contentExtraction();
    //std::cout << "======= Score result =======" << std::endl;
    *sum = droot.calculate_score(gold_buf, gold_path);

    // free memory
    delete [] buf;
    delete [] gold_buf;
    delete_node(root);
    tidyBufFree( &output );
    tidyBufFree( &errbuf );
    tidyRelease( doc );
}


int main (int argc, char **argv) {

    std::string root_path = "/home/ianliu/develope/maintex_Extraction/CETD/BBC/";
    bfs::path path_gold(root_path + "gold/");
    struct Display *sum = new struct Display [101];
    std::string filenames;
    std::string path_origin = root_path + "original/";
    std::string path_origin_file;
    int i = 0;


    for(bfs::directory_iterator git = bfs::directory_iterator(path_gold); git != bfs::directory_iterator(); ++ git) {
        // find file in orginal that is corresponding to file in gold
        filenames = git->path().filename().string();
        path_origin_file =  path_origin + filenames.substr(0, filenames.find(".")) + ".htm";
        std::cout << path_origin_file << std::endl;

        maintexExtraction(path_gold.string() + git->path().filename().string(), 
        path_origin_file, &sum[i]);
        ++ i;
        if(i > 30) break;
    }

    std::cout << std::setw(7) << std::setw(10) << "precision" << std::setw(10) 
    << "recall" << std::setw(10) << "F1 score" << std::endl;
    for(int i = 0; i < 30; i++ ) {
        std::cout << std::setw(7)  << std::setw(10)
        << sum[i].precision << std::setw(10) << sum[i].recall << std::setw(10) 
        << sum[i].F1_score << std::endl;
    }

    return 0;
}

