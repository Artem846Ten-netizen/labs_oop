#include <string>
#include <vector>
#include <stdexcept>

class BitArray {
private:
    static const int BLOCK = sizeof(unsigned long) * 8; // bits in one block
    std::vector<unsigned long> data;
    int numBits;

    bool getBit(int i) const {
        return (data[i / BLOCK] >> (i % BLOCK)) & 1;
    }

    void setBit(int i, bool val) {
        unsigned long mask = 1UL << (i % BLOCK);
        if (val) {
            data[i / BLOCK] |= mask;
        } else {
            data[i / BLOCK] &= ~mask;
        }
    }

    void checkIndex(int i) const {
        if (i < 0 || i >= numBits) {
            throw std::out_of_range("index out of range");
        }
    }

    void checkSize(const BitArray& b) const {
        if (numBits != b.numBits) {
            throw std::invalid_argument("arrays have different sizes");
        }
    }

public:
    // Creates an empty array.
    BitArray() {
        numBits = 0;
    }

    // Nothing to free: vector frees memory itself.
    ~BitArray() {
    }

    // Creates num_bits bits, the first bits are taken from value.
    explicit BitArray(int num_bits, unsigned long value = 0) {
        if (num_bits < 0) {
            throw std::invalid_argument("negative size");
        }
        numBits = 0;
        resize(num_bits);
        for (int i = 0; i < num_bits && i < BLOCK; i++) {
            setBit(i, (value >> i) & 1);
        }
    }

    // Copy constructor.
    BitArray(const BitArray& b) {
        data = b.data;
        numBits = b.numBits;
    }

    void swap(BitArray& b) {
        BitArray temp = *this;
        *this = b;
        b = temp;
    }

    // Assignment.
    BitArray& operator=(const BitArray& b) {
        data = b.data;
        numBits = b.numBits;
        return *this;
    }

    // Changes the size, new bits get value.
    void resize(int num_bits, bool value = false) {
        if (num_bits < 0) {
            throw std::invalid_argument("negative size");
        }
        int oldSize = numBits;
        data.resize((num_bits + BLOCK - 1) / BLOCK, 0);
        numBits = num_bits;
        for (int i = oldSize; i < numBits; i++) {
            setBit(i, value);
        }
    }

    // Removes all bits.
    void clear() {
        data.clear();
        numBits = 0;
    }

    // Adds a bit to the end.
    void push_back(bool bit) {
        resize(numBits + 1, bit);
    }

    // Bitwise operations, arrays must have the same size.
    BitArray& operator&=(const BitArray& b) {
        checkSize(b);
        for (int i = 0; i < numBits; i++) {
            setBit(i, getBit(i) && b.getBit(i));
        }
        return *this;
    }

    BitArray& operator|=(const BitArray& b) {
        checkSize(b);
        for (int i = 0; i < numBits; i++) {
            setBit(i, getBit(i) || b.getBit(i));
        }
        return *this;
    }

    BitArray& operator^=(const BitArray& b) {
        checkSize(b);
        for (int i = 0; i < numBits; i++) {
            setBit(i, getBit(i) != b.getBit(i));
        }
        return *this;
    }

    // Shift left: bit i moves to i + n, empty places get 0.
    BitArray& operator<<=(int n) {
        if (n < 0) {
            throw std::invalid_argument("negative shift");
        }
        for (int i = numBits - 1; i >= 0; i--) {
            if (i - n >= 0) {
                setBit(i, getBit(i - n));
            } else {
                setBit(i, false);
            }
        }
        return *this;
    }

    // Shift right: bit i moves to i - n, empty places get 0.
    BitArray& operator>>=(int n) {
        if (n < 0) {
            throw std::invalid_argument("negative shift");
        }
        for (int i = 0; i < numBits; i++) {
            if (i + n < numBits) {
                setBit(i, getBit(i + n));
            } else {
                setBit(i, false);
            }
        }
        return *this;
    }

    BitArray operator<<(int n) const {
        BitArray result = *this;
        result <<= n;
        return result;
    }

    BitArray operator>>(int n) const {
        BitArray result = *this;
        result >>= n;
        return result;
    }

    // Sets bit n to val.
    BitArray& set(int n, bool val = true) {
        checkIndex(n);
        setBit(n, val);
        return *this;
    }

    // Sets all bits to 1.
    BitArray& set() {
        for (int i = 0; i < numBits; i++) {
            setBit(i, true);
        }
        return *this;
    }

    // Sets bit n to 0.
    BitArray& reset(int n) {
        return set(n, false);
    }

    // Sets all bits to 0.
    BitArray& reset() {
        for (int i = 0; i < numBits; i++) {
            setBit(i, false);
        }
        return *this;
    }

    // True if at least one bit is 1.
    bool any() const {
        return count() > 0;
    }

    // True if all bits are 0.
    bool none() const {
        return count() == 0;
    }

    // Returns inverted copy.
    BitArray operator~() const {
        BitArray result = *this;
        for (int i = 0; i < numBits; i++) {
            result.setBit(i, !getBit(i));
        }
        return result;
    }

    // Number of bits equal to 1.
    int count() const {
        int result = 0;
        for (int i = 0; i < numBits; i++) {
            if (getBit(i)) {
                result++;
            }
        }
        return result;
    }

    // Value of bit i.
    bool operator[](int i) const {
        checkIndex(i);
        return getBit(i);
    }

    int size() const {
        return numBits;
    }

    bool empty() const {
        return numBits == 0;
    }

    // String of 0 and 1, the highest bit goes first: BitArray(5, 6) -> "00110".
    std::string to_string() const {
        std::string s = "";
        for (int i = numBits - 1; i >= 0; i--) {
            if (getBit(i)) {
                s += '1';
            } else {
                s += '0';
            }
        }
        return s;
    }
};

bool operator==(const BitArray& a, const BitArray& b) {
    return a.to_string() == b.to_string();
}

bool operator!=(const BitArray& a, const BitArray& b) {
    return !(a == b);
}

BitArray operator&(const BitArray& b1, const BitArray& b2) {
    BitArray result = b1;
    result &= b2;
    return result;
}

BitArray operator|(const BitArray& b1, const BitArray& b2) {
    BitArray result = b1;
    result |= b2;
    return result;
}

BitArray operator^(const BitArray& b1, const BitArray& b2) {
    BitArray result = b1;
    result ^= b2;
    return result;
}
