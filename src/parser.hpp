#ifndef parser_hpp
#define parser_hpp

#include <stdio.h>
#include <string>
#include <map>

using namespace std;

typedef struct Instruction_t {
public:
    string operation;
    string url;
    string version;
    map<string, string> kvPairs;
    bool valid;
} Instruction;

class Parser {
public:
    static Instruction parse(string insstr);
};
#endif /* parser_hpp */
