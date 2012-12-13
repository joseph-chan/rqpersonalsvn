#include "ac.h"


using namespace std;

void Aho_Corasick::clear() {
        _node.resize(1);
        _node[Root].id = None;
        _node[Root].fail_off = None;
        _node[Root].child.clear();
        _word_count = 0;
    }


void Aho_Corasick::insert(const char* str, int id) {
    int off = Root;
    for (; *str != '\0'; str++) {
        const char& charactor = *str;
        if (_node[off].child.count(charactor) == 0) {
            _node[off].child[charactor] = _node.size();
            _node.push_back(Node());
            _node.back().id = None;
        }
        off = _node[off].child[charactor];
    }
    _node[off].id = id;
}

void Aho_Corasick::build_automata() {
    std::queue<int> que;
    que.push(Root);

	while (!que.empty()) {
        int father = que.front();
        que.pop();
        for (std::map<char, int>::const_iterator it = _node[father].child.begin();
                it != _node[father].child.end(); ++it) {
            char charactor = it->first;
            int child = it->second;
            if (father == Root) {
                _node[child].fail_off = Root;
            } else {
                int next = _node[father].fail_off;
                while (next != None) {
                    if (_node[next].child.count(charactor)) {
                        int next_child = _node[next].child[charactor]; 
                        _node[child].fail_off = next_child;
                        if (_node[child].id == None) {
                            _node[child].id = _node[next_child].id;
                        }
                        break;
                    } else {
                        next = _node[next].fail_off;
                    }
                }
                if (next == None) {
                    _node[child].fail_off = Root;
                }
            }
            que.push(child);
        }
    }
}

std::set<int> Aho_Corasick::get_match(const char* str) {
    std::set<int> ret;
    for (int off = Root; *str != '\0'; str++) {
        char charactor = *str;
        while (off != Root && _node[off].child.count(charactor) == 0) {
            off = _node[off].fail_off;
        }
        if (_node[off].child.count(charactor) == 0) {
            continue;
        }
        off = _node[off].child[charactor];
        for (int back_off = off; _node[back_off].id != None && back_off != Root;
                back_off = _node[back_off].fail_off) {
            ret.insert(_node[back_off].id);
        }
    }
    return ret;
}

std::multiset<pair<int,int>,setcomp> Aho_Corasick::get_match_off(const char* str) {
    std::multiset<pair<int,int>,setcomp> ret;
	const char *orig_str = str;
    for (int off = Root; *str != '\0'; str++) {
        char charactor = *str;
        while (off != Root && _node[off].child.count(charactor) == 0) {
            off = _node[off].fail_off;
        }
        if (_node[off].child.count(charactor) == 0) {
            continue;
        }
        off = _node[off].child[charactor];
        for (int back_off = off; _node[back_off].id != None && back_off != Root;
                back_off = _node[back_off].fail_off) {
            ret.insert(pair<int,int>(_node[back_off].id, str-orig_str));
        }
    }
    return ret;
}


const int Aho_Corasick::Root;

void show_set(const std::set<int>& st ,map<int,string> & out) {
    int a;
    for (std::set<int>::const_iterator it = st.begin();
            it != st.end(); ++it) {
        a=*it;
        std::cout << "[" << *it << "]" << out[a];
    }
    std::cout << std::endl;
}
void show_set(const std::set<int>& st ) {
    for (std::set<int>::const_iterator it = st.begin();
            it != st.end(); ++it) {
        std::cout << "[" << *it << "]" ;
    }
    std::cout << std::endl;
}

/*
int main() {
    Aho_Corasick ac;
    Aho_Corasick ac2;
    std::ifstream fin("tag2.utf8");
    std::string line;
    int second;
    char buff[10240];
    char word[64];
    char prop[64];
    map<int,string> out;
    while(!fin.eof())
    {
        getline(fin,line);
        snprintf(buff,10240,"%s",line.c_str());
        sscanf(buff, "%s%s", word, prop);
        second = atoi(prop);
        cout << word << " " << prop << endl;
        
        ac.insert(word,second);
        out[atoi(prop)]= line;
    }
    ac.build_automata();

    std::cout << "end init" << std::endl;

	const char *query = "大明星大明星从军记明星经纪人大明星运动会";
    show_set(ac.get_match(query),out);
    ac2.insert("明星", 1);
    ac2.insert("大明星", 2);
    ac2.insert("大明星从军记", 3);
    ac2.insert("大明星运动会", 4);
    ac2.insert("明星经纪人", 5);

	const char *query2 = "大明星大明星从军记明星经纪人大明星运动会";
    ac2.build_automata();
    show_set(ac2.get_match(query2));






    ac.insert("she", 1);
    ac.insert("he", 2);
    ac.build_automata();
    show_set(ac.get_match("he is cool"));
    show_set(ac.get_match("she is cool"));

    ac.clear();
    ac.insert("she", 1);
    ac.insert("he", 2);
    ac.insert("say", 3);
    ac.insert("shr", 4);
    ac.insert("her", 5);
    ac.build_automata();
    show_set(ac.get_match("yasherhs"));

    return 0;
}
*/
