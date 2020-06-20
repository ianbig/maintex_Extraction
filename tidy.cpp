#define BOOST_FILESYSTEM_VERSION 3

#include <tidy/tidy.h>
#include <tidy/buffio.h>
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


void Tagcount(pt::ptree &root) {

    for(auto it = root.begin(); it != root.end(); it++) {
        if(stack.find(it->first) != stack.end()) {
            stack.find(it->first)->second += 1;
        }
        else {
            stack.insert(std::pair<std::string, int>(it->first, 1));
        }
        Tagcount(it->second);
    }
}

void maintexExtraction(std::string html_path, std::string gold_path) {
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
    std::cout << "========Create Tree=========" << std::endl;
    root = create_dom_tree(tree, "root"); // root means on top of tree = first call of function
    DOM_tree droot(root);
    std::cout << "Tree Built Complete" << std::endl;
    std::cout << "========Extract Content=========" << std::endl;
    droot.contentExtraction();
    std::cout << "======= Score result =======" << std::endl;
    droot.calculate_score(gold_buf);

    // free memory
    delete [] buf;
    delete [] gold_buf;
    delete_node(root);
    tidyBufFree( &output );
    tidyBufFree( &errbuf );
    tidyRelease( doc );
}


int main (int argc, char **argv) {

    std::string root_path = "./CETD/BBC/";
    bfs::path path_gold(root_path + "gold/");
    bfs::path path_origin(root_path + "original/");

    std::cout << path_gold << std::endl;
    bfs::directory_iterator oit = bfs::directory_iterator(path_origin);
    for(bfs::directory_iterator git = bfs::directory_iterator(path_gold); git != bfs::directory_iterator(); ++ git) {
        maintexExtraction(git->path().filename().string(), oit->path().filename().string());
        ++ oit;
    }

    return 0;
}

