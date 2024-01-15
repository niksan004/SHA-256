#include <iostream>
#include <bitset> // only using the bitset data type because it is much more memory-efficient


const unsigned short  CHUNK_SIZE = 512;
const unsigned short BINARY_NUM_SIZE = 8;   // used to convert char to binary number
const unsigned short  BIG_ENDIAN_SIZE = 64;  // big endian number (representing length of input string) in bits

const unsigned short  BINARY_WORD_SIZE = 32; // used in the second step where each word is 32 bits
const unsigned short  BINARY_WORD_ARR_SIZE = 64; // size of the array which contains the 32 bit words


typedef std::bitset<BINARY_NUM_SIZE> binary8Bit;
typedef std::bitset<BINARY_WORD_SIZE> binary32Bit;
typedef std::bitset<BIG_ENDIAN_SIZE> binary64Bit;


struct dynamic8BitBinaryArr
{
    size_t capacity = CHUNK_SIZE / BINARY_NUM_SIZE;
    size_t length = 0;
    binary8Bit* arr = new binary8Bit[capacity];
};

struct dynamic32BitBinaryArr
{
    size_t capacity = BINARY_WORD_ARR_SIZE;
    size_t length = 0;
    binary32Bit* arr = new binary32Bit[capacity];
};


int modifyZeroedIdndexes(dynamic32BitBinaryArr& binaryArr32Bit);
bool fullAdder(bool b1, bool b2, bool& carry);
binary32Bit bitsetAdd(binary32Bit& x, binary32Bit& y);
binary32Bit leftRotate(binary32Bit binaryNum, unsigned int rotation);
binary32Bit rightRotate(binary32Bit binaryNum, unsigned int rotation);
dynamic32BitBinaryArr convert8BitTo32BitBinaryArr(dynamic8BitBinaryArr& binaryArr);
binary32Bit createEntry(int startIdx, int endIdx, dynamic8BitBinaryArr& binaryArr);
int addBigEndian(dynamic8BitBinaryArr& binaryArr);
int stringToBinary(char* str, dynamic8BitBinaryArr& binaryArr);
binary8Bit decimalToBinary8Bit(int decimal);
binary64Bit decimalToBinary64Bit(int decimal);
void updateArrayCapacity(dynamic8BitBinaryArr& binaryArr);
void copyElementsIntoNewArr(const dynamic8BitBinaryArr& binaryArr, binary8Bit* newArr);
void printArray(dynamic32BitBinaryArr& binaryArr);
void printArray(dynamic8BitBinaryArr& binaryArr);


int main()
{
    char text[] = "hello world";
    dynamic8BitBinaryArr binaryArr;

    // convert inputted string into an array of binary numbers
    stringToBinary(text, binaryArr);

    // add big endian representing string length in binary at the end of the binary array
    addBigEndian(binaryArr);

    // append 1 after the last binary number 
    binaryArr.arr[binaryArr.length - 1] = decimalToBinary8Bit(128);

    // hash values (squares of first 8 primes: 2, 3, 5, 7, 11, 13, 17, 19)
    int h0 = 0x6a09e667;
    int h1 = 0xbb67ae85;
    int h2 = 0x3c6ef372;
    int h3 = 0xa54ff53a;
    int h4 = 0x510e527f;
    int h5 = 0x9b05688c;
    int h6 = 0x1f83d9ab;
    int h7 = 0x5be0cd19;

    // convert old arr to new arr with 64 in number 32 bit words
    dynamic32BitBinaryArr binaryArr32Bit = convert8BitTo32BitBinaryArr(binaryArr);

    // modify the zero-ed indexes at the end of the array
    modifyZeroedIdndexes(binaryArr32Bit);

    printArray(binaryArr32Bit);

    // free memory in heap
    delete[] binaryArr.arr;
    delete[] binaryArr32Bit.arr;
}


int modifyZeroedIdndexes(dynamic32BitBinaryArr& binaryArr32Bit)
{
    //                                  = 16
    for (unsigned short i = CHUNK_SIZE / BINARY_WORD_SIZE; i < BINARY_WORD_ARR_SIZE; ++i)
    {
        // for every iteration:
        // s0 = (w[i - 15] rightrotate 7) xor (w[i - 15] rightrotate 18) xor (w[i - 15] rightshift 3)
        // s1 = (w[i - 2] rightrotate 17) xor (w[i - 2] rightrotate 19) xor (w[i - 2] rightshift 10)
        // w[i] = w[i - 16] + s0 + w[i - 7] + s1

        binary32Bit s0 = rightRotate(binaryArr32Bit.arr[i - 15], 7) ^ rightRotate(binaryArr32Bit.arr[i - 15], 18) ^ (binaryArr32Bit.arr[i - 15] >> 3);
        binary32Bit s1 = rightRotate(binaryArr32Bit.arr[i - 2], 17) ^ rightRotate(binaryArr32Bit.arr[i - 2], 19) ^ (binaryArr32Bit.arr[i - 2] >> 10);

        binary32Bit firstAddition = bitsetAdd(binaryArr32Bit.arr[i - 16], s0);
        binary32Bit secondAddition = bitsetAdd(firstAddition, binaryArr32Bit.arr[i - 7]);

        binaryArr32Bit.arr[i] =  bitsetAdd(secondAddition, s1);
    }

    return 1;
}

bool fullAdder(bool b1, bool b2, bool& carry)
{
    bool sum = (b1 ^ b2) ^ carry;
    carry = (b1 && b2) || (b1 && carry) || (b2 && carry);
    return sum;
}

binary32Bit bitsetAdd(binary32Bit& x, binary32Bit& y)
{
    bool carry = false;
    binary32Bit ans;

    for (int i = 0; i < BINARY_WORD_SIZE; ++i) { ans[i] = fullAdder(x[i], y[i], carry); }

    return ans;
}

binary32Bit leftRotate(binary32Bit binaryNum, unsigned int rotation)
{
    return (binaryNum << rotation) | (binaryNum >> (BINARY_WORD_SIZE - rotation));
}

binary32Bit rightRotate(binary32Bit binaryNum, unsigned int rotation)
{
    return (binaryNum >> rotation) | (binaryNum << (BINARY_WORD_SIZE - rotation));
}

dynamic32BitBinaryArr convert8BitTo32BitBinaryArr(dynamic8BitBinaryArr& binaryArr)
{
    dynamic32BitBinaryArr newBinaryArr;

    // create entry
    for (int i = 0; i <= binaryArr.capacity - 4; i += 4)
    {
        binary32Bit entry = createEntry(i, i + 4, binaryArr);
        newBinaryArr.arr[newBinaryArr.length++] = entry;
    }

    return newBinaryArr;
}

binary32Bit createEntry(int startIdx, int endIdx, dynamic8BitBinaryArr& binaryArr)
{
    binary32Bit entry;
    int entryIdx = BINARY_WORD_SIZE;

    for (int i = startIdx; i < endIdx; ++i)
    {
        for (int j = BINARY_NUM_SIZE - 1; j >= 0; --j)
        {
            entry[--entryIdx] = binaryArr.arr[i][j];
        }
    }

    return entry;
}

int addBigEndian(dynamic8BitBinaryArr& binaryArr)
{
    size_t bigEndian = size_t(binaryArr.length * BINARY_NUM_SIZE);

    for (int i = BIG_ENDIAN_SIZE / BINARY_NUM_SIZE - 1; i >= 0; --i)
    {
        binaryArr.arr[binaryArr.capacity - i - 1] = (bigEndian >> (BINARY_NUM_SIZE * i));
    }

    return 1;
}

binary64Bit decimalToBinary64Bit(int decimal)
{
    binary64Bit res = 0;

    for (int i = 0; i < BIG_ENDIAN_SIZE; ++i)
    {
        int decimalShiftedCopy = decimal >> i;
        binary64Bit mask = uint64_t(1) << i;
        if (decimalShiftedCopy & 1) { res = res | mask; }
    }

    return res;
}

int stringToBinary(char* str, dynamic8BitBinaryArr& binaryArr)
{
    if (str == nullptr || binaryArr.arr == nullptr) { return 0; }

    while (*str != '\0') 
    { 
        if (binaryArr.length == binaryArr.capacity - BIG_ENDIAN_SIZE / BINARY_NUM_SIZE) { updateArrayCapacity(binaryArr); }
        binaryArr.arr[binaryArr.length++] = decimalToBinary8Bit(*(str++));
    }
    ++binaryArr.length;

    return 1;
}

binary8Bit decimalToBinary8Bit(int decimal)
{
    binary8Bit res = 0;

    for (int i = 0; i < BINARY_NUM_SIZE; ++i)
    {
        int decimalShiftedCopy = decimal >> i;
        binary8Bit mask = uint64_t(1) << i;
        if (decimalShiftedCopy & 1) { res = res | mask; }
    }

    return res;
}

void updateArrayCapacity(dynamic8BitBinaryArr& binaryArr)
{
    binaryArr.capacity = binaryArr.capacity + (CHUNK_SIZE / BINARY_NUM_SIZE);

    binary8Bit* newArr = new binary8Bit[binaryArr.capacity];
    copyElementsIntoNewArr(binaryArr, newArr);
    delete[] binaryArr.arr;
    binaryArr.arr = newArr;
}

void copyElementsIntoNewArr(const dynamic8BitBinaryArr& binaryArr, binary8Bit* newArr)
{
    for (size_t i = 0; i < binaryArr.length; ++i) { newArr[i] = binaryArr.arr[i]; }
}

void printArray(dynamic32BitBinaryArr& binaryArr)
{
    for (int i = 0; i < binaryArr.capacity; ++i) { std::cout << binaryArr.arr[i] << ' '; }
}

void printArray(dynamic8BitBinaryArr& binaryArr)
{
    for (int i = 0; i < binaryArr.capacity; ++i) { std::cout << binaryArr.arr[i] << ' '; }
}
