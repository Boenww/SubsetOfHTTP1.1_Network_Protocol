#include "parser.hpp"

static const std::string SEP = "\r\n";
static const std::string SPACE = " ";
static const std::string COLON_SPACE = ": ";

Instruction Parser::parse(string insstr) {
    Instruction inst;
    
    size_t l = 0;
    size_t r = insstr.find(SEP);
    string initLine = insstr.substr(0, r);
    
    //fetch operation
    size_t m = initLine.find(SPACE);
    if (m == string::npos) {
        inst.valid = false;
        return inst;
    }
    else {
        inst.operation = initLine.substr(0, m);
    }
    //url
    l = m + SPACE.size();
    m = initLine.find(SPACE, l);
    if (m == string::npos){
        inst.valid = false;
        return inst;
    }
    else {
        inst.url = insstr.substr(l, m-l);
    }
    //version
    l = m + SPACE.size();
    if (l == r){
        inst.valid = false;
        return inst;
    }
    else {
        inst.version = initLine.substr(l, r-l);
    }
    //kvpairs
    l = 0;
    string KVPairs  = insstr.substr(r+SEP.size(), insstr.size()-r-SEP.size());
    while((l < KVPairs.size()) && ((r=KVPairs.find(SEP, l)) != string::npos))
    {
        string curLine = KVPairs.substr(l, r-l);
        unsigned long flg = curLine.find(COLON_SPACE);
        if (flg != string::npos) {
            string key = curLine.substr(0, flg);
            string value = curLine.substr(flg+COLON_SPACE.size(), curLine.size()-flg-COLON_SPACE.size());
            inst.kvPairs[key] = value;
        }
        l = r + COLON_SPACE.size();
    }
    
    inst.valid = true;
    return inst;
}
