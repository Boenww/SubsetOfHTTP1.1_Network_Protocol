#ifndef FRAMER_HPP
#define FRAMER_HPP

#include <iostream>
#include <deque>

using namespace std;

class Framer {
public:
    Framer();
    
    void append(std::string chars);
    
    // Does this buffer contain at least one complete message?
    bool hasMessage() const;
    
    // Returns the first instruction
    string topMessage() const;
    
    // Removes the first instruction
    void popMessage();
    
    // prints the string to an ostream (useful for debugging)
    void printToStream(std::ostream& stream) const;
    
protected:
    deque<string> msgQueue;
    string lastIncompleteMsg;
    
};

#endif // FRAMER_HPP
