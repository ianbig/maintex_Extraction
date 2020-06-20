# ifndef TREE_H
# define TREE_H

#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <boost/property_tree/ptree.hpp>

#define INITSIZE 10
#define UP 0
#define LEFT 1
#define UPPER_LEFT 2

namespace pt = boost::property_tree;

class DOM_tree;

struct Display {
    std::string path;
    double F1_score;
    double recall;
    double precision;
};

class tree_node {
    private:
        //size_t child_size;
        size_t max_child_size;
        //tree_node **children;
    public:
        tree_node **children;
        std::string tagname;
        double char_count;
        double tag_count;
        double text_density;
        size_t child_size;
        std::string value;
        tree_node();
        tree_node(const tree_node &copy);
        ~tree_node();
        tree_node& operator= (const tree_node &rhs);
        void push_back(tree_node *node);
    friend class DOM_tree;
    };

class DOM_tree {
    private:
        //double threshold;
    public:
        tree_node *root;
        std::string content_buffer;
        // create tree and delete not necessary node e.g.
        // script
        // no script
        // <xmlcomment>
        // <xmlattr>
        // <xml:lang>
        DOM_tree(tree_node *rroot);
        // cleanup DOM_tree
        void contentExtraction();
        void traverse(tree_node *node);
        void get_threshold(tree_node *node); // have memory bug
        void extract_maintex(tree_node *node);
        struct Display calculate_score(char *golden_text, std::string gold_path);
        double threshold;
};

struct lcs_info {
    int data;
    int direction;
};

// dom tree is built recursievely
// ptree is recursively parse
// and the corresponding node is built according to current ptree node
// comment, syle, script is removed from dom_tree since most news article do not need javascript to show content
// @ node is ptree node @tagnaem is current tagname in node 
tree_node* create_dom_tree(pt::ptree node, std::string tagname);
// 
void delete_node(tree_node *node);
// find out the longest common subsequence betweem s1 and s2
int lcs(std::string s1, std::string s2);
// find out max lcs between lcs_1 and lcs2
// and assign value and where it gets the value(i.e. LEFT, UP, or UPPER_LEFT) into lcs_assign
void lcs_max(struct lcs_info *lcs_assign, struct lcs_info *lcs_1, struct lcs_info *lcs_2);
// print out the lcs string @lcs_ptr as dp table, @s1_length = one of the length in compare string
// @ s2_length = one of the length in compare string
void lcs_backtrace(struct lcs_info **lcs_ptr, size_t s1_length, size_t s2_length, std::string s1);


# endif