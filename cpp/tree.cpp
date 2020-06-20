#include "tree.h"

tree_node::tree_node() {
    char_count = 0;
    tag_count = 0;
    text_density = 0.0;
    child_size = 0;
    max_child_size = INITSIZE;
    children = new tree_node*[INITSIZE];
}

tree_node::~tree_node() {

}

tree_node::tree_node(const tree_node &copy) {
    this->tagname = copy.tagname;
    this->char_count = copy.char_count;
    this->tag_count = copy.tag_count;
    this->text_density = copy.text_density;
    this->children = copy.children;
}

tree_node& tree_node::operator=(const tree_node &rhs) {
    if(this == &rhs) return *this;
    this->tagname = rhs.tagname;
    this->char_count = rhs.char_count;
    this->tag_count = rhs.tag_count;
    this->text_density = rhs.text_density;
    this->children = rhs.children;
    return *this;
}

void tree_node::push_back(tree_node *node) {
    if(child_size + 1 > max_child_size) {
       max_child_size = child_size * 2;
       tree_node **tmp_tree = children;
       children = new tree_node*[max_child_size];
       for(int i = 0; i < child_size; i++ ) {
           children[i] = tmp_tree[i];
       }
       delete [] tmp_tree;
    } // reallocate
    children[child_size] = node;
    child_size += 1;
    return;
}

DOM_tree::DOM_tree(tree_node *rroot) {
    this->root = rroot;
    content_buffer = "";
}


void DOM_tree::contentExtraction() {
    get_threshold(root);
    extract_maintex(root);
}

// Put the threshold value of body into threhold
// By doing this, I could save the memory space for tmp_threshold
// which is use as intermediate variable for passing value back from recursive
void DOM_tree::get_threshold(tree_node *node) {
    tree_node *tmp_node;
    if(node->tagname.compare("body") == 0) {
        threshold = node->text_density; 
        return;
    }
    for(int i = 0; i < node->child_size; i++) {
        tmp_node = node->children[i];
        get_threshold(tmp_node);
    }
    return;
}

void DOM_tree::extract_maintex(tree_node *node) {
    tree_node *tmp_node;
    std::string tagname = node->tagname;

    if(node->text_density < threshold && node->tagname != "root" && node->tagname != "html") return;

    else {
        if(
            tagname == "p" || tagname == "strong" || tagname == "i" || tagname == "b" ||
            tagname == "em" || tagname == "mark" || tagname == "small" || tagname == "del" ||
            tagname ==  "ins" || tagname == "sub" || tagname == "sup" || tagname == "h1" || 
            tagname == "h2" || tagname == "h3" || tagname == "h4" || tagname == "h5" ||
            tagname == "h6") {
                content_buffer += node->value;
        }
        for(int i = 0; i < node->child_size; i++) {
            tmp_node = node->children[i];
            extract_maintex(tmp_node);
        }
    }
}


void DOM_tree::traverse(tree_node *node) {
    tree_node* tmp_child;
    std::cout << "Tag name: " << node->tagname << 
        " Text Density: " << node->text_density << 
        " Text Count: " << node->char_count << " Tag Count: " 
        << node->tag_count << std::endl;
    for(int i = 0; i < node->child_size; i++) {
        tmp_child = node->children[i];
        traverse(tmp_child);
    }
}

struct  Display DOM_tree::calculate_score(char *golden_text, std::string gold_path) {
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

tree_node* create_dom_tree(pt::ptree node, std::string tagname) {
    tree_node *parent = new tree_node;
    tree_node *child;
    int tmp_char_count = 0;

    parent->tagname = tagname;

    if(
        tagname == "p" || tagname == "strong" || tagname == "i" || tagname == "b" ||
        tagname == "em" || tagname == "mark" || tagname == "small" || tagname == "del" ||
        tagname ==  "ins" || tagname == "sub" || tagname == "sup" || tagname == "h1" || 
        tagname == "h2" || tagname == "h3" || tagname == "h4" || tagname == "h5" ||
        tagname == "h6") {
            parent->value = node.data();
            tmp_char_count = node.data().length();
        }

    for(auto it = node.begin(); it != node.end(); it++) {
        if(
            it->first != "script" && it->first != "noscript" && 
            it->first != "style" && it->first != "<xmlcomment>"&&
            it->first != "<xmlattr>"  && it->first != "<xml:lang>" ) 
        {
            child = create_dom_tree(it->second, it->first); // second argument is to identify maintex's tag   
            parent->char_count += child->char_count + tmp_char_count;
            parent->tag_count += child->tag_count + 1; // include itself
            parent->push_back(child);
        }
        else continue;
    }

    // in general tag_count == 0 means hitting the bottom of tree; hence,
    // we add the char vlaue  and set tag_count = 1
    // if not doing this the leaf's (bottom of tree) value is unable to return back to parent
    // make the every node in tree have no character count
    if(parent->tag_count == 0) {
        parent->char_count += tmp_char_count;
        parent->tag_count = 1;
    }

    parent->text_density = (parent->char_count) / (parent->tag_count);
    
    return parent;
}

void delete_node(tree_node *node) {
    // tree leaf have no children hence delete node and return
    if( node->child_size == 0 ) {
        delete [] node->children;
        delete  node;
        return;
    }

    for(int i = 0; i < node->child_size; i++) {
        delete_node(node->children[i]);
    }

    delete [] node->children;
    delete node;
}

int lcs( std::string s1, std::string s2) {
    size_t s1_length = s1.length();
    size_t s2_length = s2.length();
    struct lcs_info **lcs_array;
    int i = 0, j = 0, answer = 0;

    lcs_array = new struct lcs_info*[s1_length + 1];

    for(i = 0; i < s1_length + 1; i++) 
    {
        lcs_array[i] = new struct lcs_info[s2_length + 1];
    }

    // initialize lcs_array
    for( i = 0; i < s1_length + 1; i++)
        for( j = 0; j < s2_length + 1; j++) {
            lcs_array[i][j].data = 0;
        }
    
    // string and lcs_array exist 1 element difference(lcs_array considered null character)
    // hence index of lcs_array have to plus one to be correspond to the string
    // for example abc and a, we got
    //      null a b c
    // null 0    0 0 0
    // a    0    1 1 1    
    for( i = 0; i < s1_length; i++) {
        for ( j = 0; j < s2_length; j++) {
            if(s1[i] == s2[j] ) {
                lcs_array[ i + 1][j + 1].data = 1 + lcs_array[ i + 1 - 1][j + 1 - 1].data;
                lcs_array[ i + 1][j + 1].direction = UPPER_LEFT;
            }
            else {
                lcs_max(&lcs_array[ i + 1][ j + 1], &lcs_array[ i + 1 - 1][ j + 1], &lcs_array[i + 1][ j + 1 -1]);
            }
        }
    }  

    //lcs_backtrace(lcs_array, s1_length, s2_length, s1);
    answer = lcs_array[s1_length][s2_length].data;

    for(int i = 0; i < s1_length + 1; i++) {
        delete [] lcs_array[i];
    }
    delete [] lcs_array;

    return answer;
}


// the purpose of passing argument as pointer is to avoid call by value
// which whould intrigate copy constructor and assignment operator
void lcs_max( struct lcs_info * lcs_assign, struct lcs_info *lcs_1, struct lcs_info *lcs_2) {
    if(lcs_1->data >= lcs_2->data) {
        lcs_assign->data = lcs_1->data;
        lcs_assign->direction = UP;
    }

    else {
        lcs_assign->data = lcs_2->data;
        lcs_assign->direction = LEFT;
    }
}

void lcs_backtrace(struct lcs_info **lcs_ptr, size_t s1_length, size_t s2_length, std::string s1) {

    if(s1_length == 0 || s2_length == 0) return;

    if(lcs_ptr[s1_length][s2_length].direction == UPPER_LEFT) {
       lcs_backtrace(lcs_ptr, s1_length - 1, s2_length - 1, s1);
       //std::cout << s1[s1_length - 1]; // since lcs_array is one row and column wider than string
    }

    else if(lcs_ptr[s1_length][s2_length].direction == LEFT) {
        lcs_backtrace(lcs_ptr, s1_length, s2_length - 1, s1);
    }

    else {
        lcs_backtrace(lcs_ptr, s1_length - 1, s2_length, s1);
    }
}