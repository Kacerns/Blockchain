#include "bitops.hpp"

int ConvertCharToASCII(char c){
    return int(c);
}
string ConvertToHex(bitset<32> bitsToConvert){
    ostringstream oss;
    oss <<hex<<bitsToConvert.to_ulong();
    string result = oss.str();

    while(result.size() <8){
        result.insert(result.begin(),'0');
    }
    return result;
}
bool fullAdder(bool b1, bool b2, bool& carry) 
{ 
    bool sum = (b1 ^ b2) ^ carry; 
    carry = (b1 && b2) || (b1 && carry) || (b2 && carry); 
    return sum; 
} 
bitset<32> bitsetAdd(const bitset<32>& x,const bitset<32>& y) 
{ 
    bool carry = false;
    bitset<32> ans; 
    for (int i = 0; i < 32; i++) { 
        ans[i] = fullAdder(x[i], y[i], carry); 
    } 
    return ans; 
} 