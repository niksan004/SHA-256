﻿/**
*
* Solution to course project #06
* Introduction to programming course
* Faculty of Mathematics and Informatics of Sofia University
* Winter semester 2023/2024
*
* @author Николай Стоянов
* @idnumber 5MI0600360
* @compiler VC
*
* file with all the code
*
*/


#include <iostream>
#include <fstream>


const unsigned short CHUNK_SIZE = 512;
const unsigned short BINARY_NUM_SIZE = 8;   // used to convert char to binary number
const unsigned short BIG_ENDIAN_SIZE = 64;  // big endian number (representing length of input string) in bits

const unsigned short BINARY_WORD_SIZE = 32; // used in the second step where each word is 32 bits
const unsigned short BINARY_WORD_ARR_SIZE = 64; // size of the array which contains the 32 bit words

const unsigned short OUTPUT_SIZE = 65; // 8 * 8 for the 8 hash values, which are 8 symbols long + 1 to add '\0' to make it a string


// each of the constants is the first 32 bits of the fractional parts of the cube roots of the first 64 primes (2 - 311).
const uint32_t roundConsts[64] = {
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2
};

// hash values (squares of first 8 primes: 2, 3, 5, 7, 11, 13, 17, 19)
uint32_t hashValues[8] = {
    0x6a09e667,
    0xbb67ae85,
    0x3c6ef372,
    0xa54ff53a,
    0x510e527f,
    0x9b05688c,
    0x1f83d9ab,
    0x5be0cd19
};


struct dynamic8BitUIntArr
{
    size_t capacity = CHUNK_SIZE / BINARY_NUM_SIZE;
    size_t length = 0;
    uint8_t* arr = new uint8_t[capacity];
};

struct dynamic32BitUIntArr
{
    size_t capacity = BINARY_WORD_ARR_SIZE;
    size_t length = 0;
    uint32_t* arr = new uint32_t[capacity];
};

struct string
{
    size_t capacity = BINARY_WORD_ARR_SIZE;
    size_t length = 0;
    char* arr = new char[capacity];
};


bool compareHash(char firstHash[OUTPUT_SIZE], char secondHash[OUTPUT_SIZE]);
int readFromFile(string& str, char* fileName);
int appendToStr(string& str, char* strToAppend, size_t length);
void updateStrCapacity(string& str);
void copyElementsIntoNewArr(const string& str, char* newArr);
int writeToFile(char* strToWrite, char* fileName);
int SHA256(const char* input, char output[OUTPUT_SIZE]);
int toString(uint32_t* hashValues, char* str);
char getCharInHexadecAt(uint32_t hexadecNum, int idx);
void modifyHashValues(uint32_t* compressedValues);
void compress(dynamic32BitUIntArr& arrMessageSchedule, uint32_t* valuesForCompression);
int modifyZeroedIndexes(dynamic32BitUIntArr& arrMessageSchedule);
uint32_t leftRotate(uint32_t binaryNum, unsigned int rotation);
uint32_t rightRotate(uint32_t binaryNum, unsigned int rotation);
int convert8BitTo32BitArr(dynamic8BitUIntArr& arr, dynamic32BitUIntArr& arrMessageSchedule, size_t startIdx);
uint32_t createEntry(int startIdx, int endIdx, dynamic8BitUIntArr& arr);
int addLengthAtEnd(dynamic8BitUIntArr& arr);
int stringToArr(const char* str, dynamic8BitUIntArr& binaryArr);
void updateArrayCapacity(dynamic8BitUIntArr& arr);
void copyElementsIntoNewArr(const dynamic8BitUIntArr& oldArr, uint8_t* newArr);
void initArrWithZeros(dynamic32BitUIntArr& arrMessageSchedule);
void initArrWithZeros(dynamic8BitUIntArr& arr);
void initArrWithZeros(uint8_t* arr, size_t length);
void copyArr(const uint32_t* oldArr, uint32_t* newArr, size_t length);


int main()
{
    char output[OUTPUT_SIZE];
    string input;
    string inputFileName;

    int choice = 0;

    std::cout << "What do you want to do: \n" <<
        "  write 0 to read text from file and hash it and save it in another file \n" <<
        "  write 1 to read text from file, hash it and compare it to another hash \n";

    std::cin >> choice;

    while (true)
    {
        // check if choice is inputted correctly
        if (choice != 0 && choice != 1)
        {
            std::cout << "Please enter a valid choice ";
            std::cin >> choice;
            continue;
        }

        std::cout << "Input file name: ";
        std::cin >> inputFileName.arr;

        // check if reading from file is executed correctly
        if (!readFromFile(input, inputFileName.arr))
        {
            std::cout << "File doesn't exist\n";
            continue;
        }

        if (choice == 0)
        {
            SHA256(input.arr, output);

            string outputFileName;
            std::cout << "Choose name for output file: ";
            std::cin >> outputFileName.arr;
            writeToFile(output, outputFileName.arr);

            std::cout << "successful execution";

            delete[] outputFileName.arr;
            break;
        }
        else if (choice == 1)
        {
            SHA256(input.arr, output);

            char hashToCompare[OUTPUT_SIZE];
            std::cout << "Input hash to compare to: ";
            std::cin >> hashToCompare;

            if (compareHash(output, hashToCompare)) { std::cout << "Hashes are the same"; }
            else { std::cout << "Hashes are not the same"; }

            break;
        }
    }

    delete[] input.arr;
    delete[] inputFileName.arr;
}


bool compareHash(char firstHash[OUTPUT_SIZE], char secondHash[OUTPUT_SIZE])
{
    for (size_t i = 0; i < OUTPUT_SIZE; ++i)
    {
        if (firstHash[i] >= 'a' && firstHash[i] <= 'z') { firstHash[i] = firstHash[i] - ('a' - 'A'); }
        if (secondHash[i] >= 'a' && secondHash[i] <= 'z') { secondHash[i] = secondHash[i] - ('a' - 'A'); }
        if (firstHash[i] != secondHash[i]) { return false; }
    }
    return true;
}

int readFromFile(string& str, char* fileName)
{
    const size_t stringSize = 64;
    char temp[stringSize];

    std::ifstream file(fileName);

    if (!file.good()) { return 0; } // check whether file exists

    while (!file.eof())
    {
        file.getline(temp, stringSize);
        appendToStr(str, temp, stringSize);
    }

    file.close();

    return 1;
}

int appendToStr(string& str, char* strToAppend, size_t length)
{
    for (int i = 0; i < length; ++i)
    {
        if (str.length == str.capacity) { updateStrCapacity(str); }

        str.arr[str.length++] = strToAppend[i];
    }

    return 1;
}

void updateStrCapacity(string& str)
{
    str.capacity = str.capacity + BINARY_WORD_ARR_SIZE;

    char* newArr = new char[str.capacity];
    copyElementsIntoNewArr(str, newArr);
    delete[] str.arr;
    str.arr = newArr;
}

void copyElementsIntoNewArr(const string& str, char* newArr)
{
    for (size_t i = 0; i < str.length; ++i) { newArr[i] = str.arr[i]; }
}

int writeToFile(char* strToWrite, char* fileName)
{
    std::ofstream file(fileName);
    file << strToWrite;
    file.close();

    return 1;
}

int SHA256(const char* input, char output[OUTPUT_SIZE])
{
    dynamic8BitUIntArr arr;
    initArrWithZeros(arr);

    // convert inputted string into an array of binary numbers
    stringToArr(input, arr);

    // add big endian representing string length in binary at the end of the binary array
    addLengthAtEnd(arr);

    // append 1 after the last binary number 
    arr.arr[arr.length] = 128;

    // chunk loop (for each 512 bit chink)
    size_t numOfChunks = arr.capacity * BINARY_NUM_SIZE / CHUNK_SIZE;
    for (int i = 0; i < numOfChunks; ++i)
    {
        // convert old arr to new arr with 64 in number 32 bit words
        dynamic32BitUIntArr arrMessageSchedule;
        initArrWithZeros(arrMessageSchedule);
        convert8BitTo32BitArr(arr, arrMessageSchedule, i * CHUNK_SIZE / BINARY_NUM_SIZE);

        // modify the zero-ed indexes at the end of the array
        modifyZeroedIndexes(arrMessageSchedule);

        // compression
        uint32_t valuesForCompression[8];
        copyArr(hashValues, valuesForCompression, 8);
        compress(arrMessageSchedule, valuesForCompression);

        // modify final values
        modifyHashValues(valuesForCompression);

        // free memory in heap
        delete[] arrMessageSchedule.arr;
    }

    toString(hashValues, output);

    // free memory in heap
    delete[] arr.arr;

    return 1;
}

int toString(uint32_t* hashValues, char* str)
{
    size_t strIdx = 0;

    for (int i = 0; i < 8; ++i)
    {
        for (int j = 0; j < 8; ++j)
        {
            str[strIdx++] = getCharInHexadecAt(hashValues[i], j);
        }
    }
    str[strIdx] = '\0';

    return 1;
}

char getCharInHexadecAt(uint32_t hexadecNum, int idx)
{
    int numAtIdx = 0;

    for (int i = 0; i < 4; ++i) 
    {
        numAtIdx += ((hexadecNum >> (BINARY_WORD_SIZE - (4 * idx) - i - 1)) & 1) << (3 - i);
    }

    return numAtIdx < 10 ? numAtIdx + '0' : numAtIdx - 10 + 'A';
}

void modifyHashValues(uint32_t* compressedValues)
{
    for (int i = 0; i < 8; ++i)
    {
        hashValues[i] += compressedValues[i];
    }
}

void compress(dynamic32BitUIntArr& arr, uint32_t* val)
{
    // for rach loop 0 - 63, a - h are the values in val
    // 
    // S1 = (e rightrotate 6) xor (e rightrotate 11) xor (e rightrotate 25)
    // ch = (e and f) xor ((not e) and g)
    // temp1 = h + S1 + ch + k[i] + w[i]
    // 
    // S0 = (a rightrotate 2) xor (a rightrotate 13) xor (a rightrotate 22)
    // maj = (a and b) xor (a and c) xor (b and c)
    // temp2 : = S0 + maj
    // 
    // h = g
    // g = f
    // f = e
    // e = d + temp1
    // 
    // d = c
    // c = b
    // b = a
    // a = temp1 + temp2

    for (int i = 0; i < BINARY_WORD_ARR_SIZE; ++i)
    {
        uint32_t s1 = rightRotate(val[4], 6) ^ rightRotate(val[4], 11) ^ rightRotate(val[4], 25);
        uint32_t ch = (val[4] & val[5]) ^ ((~val[4]) & val[6]);
        uint32_t temp1 = val[7] + s1 + ch + roundConsts[i] + arr.arr[i];

        uint32_t s0 = rightRotate(val[0], 2) ^ rightRotate(val[0], 13) ^ rightRotate(val[0], 22);
        uint32_t maj = (val[0] & val[1]) ^ (val[0] & val[2]) ^ (val[1] & val[2]);
        uint32_t temp2 = s0 + maj;

        val[7] = val[6];
        val[6] = val[5];
        val[5] = val[4];
        val[4] = val[3] + temp1;

        val[3] = val[2];
        val[2] = val[1];
        val[1] = val[0];
        val[0] = temp1 + temp2;
    }
}

int modifyZeroedIndexes(dynamic32BitUIntArr& dynArr)
{
    //                                  = 16
    for (unsigned short i = CHUNK_SIZE / BINARY_WORD_SIZE; i < BINARY_WORD_ARR_SIZE; ++i)
    {
        // for every iteration:
        // s0 = (w[i - 15] rightrotate 7) xor (w[i - 15] rightrotate 18) xor (w[i - 15] rightshift 3)
        // s1 = (w[i - 2] rightrotate 17) xor (w[i - 2] rightrotate 19) xor (w[i - 2] rightshift 10)
        // w[i] = w[i - 16] + s0 + w[i - 7] + s1

        uint32_t s0 = rightRotate(dynArr.arr[i - 15], 7) ^ rightRotate(dynArr.arr[i - 15], 18) ^ (dynArr.arr[i - 15] >> 3);
        uint32_t s1 = rightRotate(dynArr.arr[i - 2], 17) ^ rightRotate(dynArr.arr[i - 2], 19) ^ (dynArr.arr[i - 2] >> 10);

        dynArr.arr[i] = dynArr.arr[i - 16] + s0 + dynArr.arr[i - 7] + s1;
    }

    return 1;
}

uint32_t leftRotate(uint32_t binaryNum, unsigned int rotation)
{
    return (binaryNum << rotation) | (binaryNum >> (BINARY_WORD_SIZE - rotation));
}

uint32_t rightRotate(uint32_t binaryNum, unsigned int rotation)
{
    return (binaryNum >> rotation) | (binaryNum << (BINARY_WORD_SIZE - rotation));
}

int convert8BitTo32BitArr(dynamic8BitUIntArr& arr, dynamic32BitUIntArr& arrMessageSchedule, size_t startIdx)
{
    // create entry
    for (int i = startIdx; i <= arr.capacity - 4; i += 4)
    {
        uint32_t entry = createEntry(i, i + 4, arr);
        arrMessageSchedule.arr[arrMessageSchedule.length++] = entry;
    }

    return 1;
}

uint32_t createEntry(int startIdx, int endIdx, dynamic8BitUIntArr& arr)
{
    uint32_t entry = 0;
    unsigned short tempMultiplier = BINARY_WORD_SIZE;

    for (int i = startIdx; i < endIdx; ++i)
    {
        for (int j = BINARY_NUM_SIZE - 1; j >= 0; --j)
        {
            int mask = 1 << j;
            int masked_n = arr.arr[i] & mask;
            int bit = masked_n >> j;

            entry += bit << --tempMultiplier;
        }
    }

    return entry;
}

int addLengthAtEnd(dynamic8BitUIntArr& arr)
{
    size_t bigEndian = size_t(arr.length * BINARY_NUM_SIZE);

    for (int i = BIG_ENDIAN_SIZE / BINARY_NUM_SIZE - 1; i >= 0; --i)
    {
        arr.arr[arr.capacity - i - 1] = (bigEndian >> (BINARY_NUM_SIZE * i));
    }

    return 1;
}

int stringToArr(const char* str, dynamic8BitUIntArr& arr)
{
    if (str == nullptr || arr.arr == nullptr) { return 0; }

    while (*str != '\0') 
    { 
        if (arr.length == arr.capacity - BIG_ENDIAN_SIZE / BINARY_NUM_SIZE) { updateArrayCapacity(arr); }
        arr.arr[arr.length++] = uint8_t((*(str++)));
    }

    return 1;
}

void updateArrayCapacity(dynamic8BitUIntArr& arr)
{
    arr.capacity = arr.capacity + (CHUNK_SIZE / BINARY_NUM_SIZE);

    uint8_t* newArr = new uint8_t[arr.capacity];
    initArrWithZeros(newArr, arr.capacity);
    copyElementsIntoNewArr(arr, newArr);
    delete[] arr.arr;
    arr.arr = newArr;
}

void copyElementsIntoNewArr(const dynamic8BitUIntArr& binaryArr, uint8_t* newArr)
{
    for (size_t i = 0; i < binaryArr.length; ++i) { newArr[i] = binaryArr.arr[i]; }
}

void initArrWithZeros(dynamic32BitUIntArr& arrMessageSchedule)
{
    for (int i = 0; i < arrMessageSchedule.capacity; ++i) { arrMessageSchedule.arr[i] = 0; }
}

void initArrWithZeros(uint8_t* arr, size_t length)
{
    for (int i = 0; i < length; ++i) { arr[i] = 0; }
}

void initArrWithZeros(dynamic8BitUIntArr& arr)
{
    for (int i = 0; i < arr.capacity; ++i) { arr.arr[i] = 0; }
}

void copyArr(const uint32_t* oldArr, uint32_t* newArr, size_t length)
{
    for (size_t i = 0; i < length; ++i) { newArr[i] = oldArr[i]; }
}
