#include "framer.hpp"

using namespace std;

static const string DELIMITER = "\r\n\r\n";

Framer::Framer()
{
    lastIncompleteMsg = "";
}

void Framer::append(string chars)
{
    chars = lastIncompleteMsg + chars;
    unsigned long l = 0;
    unsigned long r;
    unsigned long n = chars.size();
    while((l<n) && ((r=chars.find(DELIMITER, l))!=string::npos))
    {
        msgQueue.push_back(chars.substr(l, r-l));
        l = DELIMITER.size() + r;
    }
    if(l<n)
        lastIncompleteMsg = chars.substr(l, n-l);
}

bool Framer::hasMessage() const
{
    return !msgQueue.empty();
}

string Framer::topMessage() const
{
    return msgQueue.front();
}

void Framer::popMessage()
{
    
    msgQueue.pop_front();
}

void Framer::printToStream(ostream& stream) const
{
    //debug
    stream << "== Print To Stream BEG ==" << endl;
    for (auto itor = msgQueue.begin(); itor != msgQueue.end(); ++itor) {
        stream << '"' << *itor << '"' << endl;
    }
    stream << "== Print To Stream END ==" << endl;
}
