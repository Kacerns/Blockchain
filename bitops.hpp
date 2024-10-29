#pragma once

#include "utils.hpp"

int ConvertCharToASCII(char c);
string ConvertToHex(bitset<32> bitsToConvert);
bool fullAdder(bool b1, bool b2, bool& carry);
bitset<32> bitsetAdd(const bitset<32>& x,const bitset<32>& y);

template<size_t N>
bitset<N> rotl( bitset<N> const& bits, unsigned count )
{
    count %= N;
    return bits << count | bits >> (N - count);
}
template<size_t N>
bitset<N> rotr( bitset<N> const& bits, unsigned count )
{
    count %= N;
    return bits >> count | bits << (N - count);
}

