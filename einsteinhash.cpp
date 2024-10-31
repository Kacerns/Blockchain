#include "einsteinhash.hpp"

// CONSTANTS // 
unsigned long long cSqr = 299792458; // Speed of light

string HashFunction(string input){

    // PLACEHOLDER VARIABLES //
    // Every value is 32 bits long and is used for calculations
    // These bit values are calculated by using Einstein's e=mc^2 formula and returning the last 32 bits of the number(not the first)
    // To avoid getting all 0 we will drop the order of magnitude of m and take the first two numbers possible
    // For example the mass of the earth is 5,972×10^24 kg we will use the number 59 in this instance
    vector<bitset<32>> planets;
    bitset<32> mercury = (3 * cSqr * cSqr);
    bitset<32> venus = (48 * cSqr * cSqr);
    bitset<32> earth = (59 * cSqr * cSqr);
    bitset<32> mars = (64 * cSqr * cSqr);
    bitset<32> jupiter = (18 * cSqr * cSqr);
    bitset<32> saturn = (56 * cSqr * cSqr); 
    bitset<32> uranus = (86 * cSqr * cSqr); 
    bitset<32> neptune = (10 * cSqr * cSqr);
    planets.push_back(mercury);
    planets.push_back(venus);
    planets.push_back(earth);
    planets.push_back(mars);
    planets.push_back(jupiter);
    planets.push_back(saturn);
    planets.push_back(uranus);
    planets.push_back(neptune);

    // || PREPARATION FOR HASHING
    vector<int> asciiValues;
    asciiValues.resize(input.size());

    transform(input.begin(), input.end(), asciiValues.begin(), ConvertCharToASCII);

    // Convert input to bits
    vector<bitset<8>> input_bits;
    for(int i = 0; i<asciiValues.size(); i++){
        input_bits.push_back(bitset<8>(asciiValues.at(i)));
    }

    // PADDING
    input_bits.push_back(bitset<8>("10000000"));
    while(input_bits.size()%32 != 0){
        input_bits.push_back(bitset<8>("00000000"));
    }
    input_bits.back() = bitset<8>(input.size());

    // || PREPARATION FOR HASHING END
    // Now we have 256 multiplicator bits of the original message, and we have 256 bits worth of placeholder variables that will be the resulting message after Einstein's hashing

    // Split the message into blocks of 256 bits
    vector<bitset<256>> splitBlocks;
    for(int i=0; i<input_bits.size()/32; i++){
        string block = "";
        for(int j=0; j<32; j++){
            block.append(input_bits.at(j + (i*32)).to_string());
        }
        splitBlocks.push_back(bitset<256>(block));
    }
    input_bits.clear();

    for(auto singleBlock : splitBlocks){
        vector<bitset<32>> equationBits;
        for(int i = 0; i<8; i++){
            string tempBits = "";
            for(int j = 0; j<32; j++){
                char bit = singleBlock[j + (i * 32)] ? '1' : '0';
                tempBits.push_back(bit);
            }
            equationBits.push_back(bitset<32>(tempBits));
        }
        for(int i = 0; i<2; i++){
            bitset<32> extraBits;
            extraBits = rotr(equationBits.at(i), 10) ^ ~equationBits.at(i+1)>>10 ^ equationBits.at(i%8)<<8;
            equationBits.push_back(extraBits);
        }
        // Bit Shifting
        // The Idea for The Einstein's Hashing Bit shifting is to have a main bitset for each iteration, for example the sun in our solar system
        for(auto mainSequence : equationBits){
            mercury = bitsetAdd((mainSequence ^ (bitsetAdd(rotr(mainSequence, 8), neptune))), mercury);
            venus = bitsetAdd(rotl(mainSequence, 15) ^ ~(bitsetAdd(rotr(mainSequence, 4), mercury)), venus);
            earth = bitsetAdd(bitsetAdd((rotr(mainSequence, 3) | ~rotl(mainSequence, 16)), venus), earth);
            mars = bitsetAdd(rotr(mainSequence, 4) ^ (rotl(mainSequence, 12) & earth), mars);
            jupiter = bitsetAdd(rotr(mainSequence, 10) | (~rotr(mainSequence, 18) & mars), jupiter);
            saturn = bitsetAdd(mainSequence & rotr(mainSequence, 16) ^ jupiter, saturn); 
            uranus = bitsetAdd(rotr(bitsetAdd(mainSequence, mainSequence), 6) ^ saturn, uranus); 
            neptune = bitsetAdd(rotl(mainSequence, 3) ^ rotr(mainSequence, 8) ^ uranus, neptune);
        }
    }
    return ConvertToHex(mercury)+ConvertToHex(venus)+ConvertToHex(earth)+ConvertToHex(mars)
        +ConvertToHex(jupiter)+ConvertToHex(saturn)+ConvertToHex(uranus)+ConvertToHex(neptune);
}