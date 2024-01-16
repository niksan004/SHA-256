#include <iostream>
#include <bitset> // only using the bitset data type because it is much more memory-efficient


const unsigned short  CHUNK_SIZE = 512;
const unsigned short BINARY_NUM_SIZE = 8;   // used to convert char to binary number
const unsigned short  BIG_ENDIAN_SIZE = 64;  // big endian number (representing length of input string) in bits

const unsigned short  BINARY_WORD_SIZE = 32; // used in the second step where each word is 32 bits
const unsigned short  BINARY_WORD_ARR_SIZE = 64; // size of the array which contains the 32 bit words

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


void modifyHashValues(uint32_t* compressedValues);
void compress(dynamic32BitUIntArr& arrMessageSchedule, uint32_t* valuesForCompression);
int modifyZeroedIndexes(dynamic32BitUIntArr& arrMessageSchedule);
uint32_t leftRotate(uint32_t binaryNum, unsigned int rotation);
uint32_t rightRotate(uint32_t binaryNum, unsigned int rotation);
int convert8BitTo32BitArr(dynamic8BitUIntArr& arr, dynamic32BitUIntArr& arrMessageSchedule);
uint32_t createEntry(int startIdx, int endIdx, dynamic8BitUIntArr& arr);
int addLengthAtEnd(dynamic8BitUIntArr& arr);
int stringToArr(char* str, dynamic8BitUIntArr& binaryArr);
void updateArrayCapacity(dynamic8BitUIntArr& arr);
void copyElementsIntoNewArr(const dynamic8BitUIntArr& oldArr, uint8_t* newArr);
void initArrWithZeros(dynamic32BitUIntArr& arrMessageSchedule);
void initArrWithZeros(dynamic8BitUIntArr& arr);
void copyArr(const uint32_t* oldArr, uint32_t* newArr, size_t length);
void printArray(uint32_t* arr, size_t length);
void printArray(dynamic32BitUIntArr& arr);
void printArray(dynamic8BitUIntArr& arr);


int main()
{
    char text[] = "hello world";
    dynamic8BitUIntArr arr;
    initArrWithZeros(arr);

    // convert inputted string into an array of binary numbers
    stringToArr(text, arr);

    // add big endian representing string length in binary at the end of the binary array
    addLengthAtEnd(arr);

    // append 1 after the last binary number 
    arr.arr[arr.length] = 128;

    // chunk loop (for each 512 bit chink)
    for (int i = 0; i < arr.capacity * BINARY_NUM_SIZE / CHUNK_SIZE; ++i)
    {
        // convert old arr to new arr with 64 in number 32 bit words
        dynamic32BitUIntArr arrMessageSchedule;
        initArrWithZeros(arrMessageSchedule);
        convert8BitTo32BitArr(arr, arrMessageSchedule);

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

    printArray(hashValues, 8);

    // free memory in heap
    delete[] arr.arr;
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

int convert8BitTo32BitArr(dynamic8BitUIntArr& arr, dynamic32BitUIntArr& arrMessageSchedule)
{
    // create entry
    for (int i = 0; i <= arr.capacity - 4; i += 4)
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

int stringToArr(char* str, dynamic8BitUIntArr& arr)
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

void initArrWithZeros(dynamic8BitUIntArr& arr)
{
    for (int i = 0; i < arr.capacity; ++i) { arr.arr[i] = 0; }
}

void copyArr(const uint32_t* oldArr, uint32_t* newArr, size_t length)
{
    for (size_t i = 0; i < length; ++i) { newArr[i] = oldArr[i]; }
}

void printArray(uint32_t* arr, size_t length)
{
    for (int i = 0; i < length; ++i)
    {
        std::cout << std::hex << arr[i] << std::endl;
    }
}

void printArray(dynamic32BitUIntArr& arr)
{
    for (int i = 0; i < arr.capacity; ++i) { std::cout << arr.arr[i] << ' '; }
}

void printArray(dynamic8BitUIntArr& arr)
{
    for (int i = 0; i < arr.capacity; ++i) { std::cout << arr.arr[i] << ' '; }
}
