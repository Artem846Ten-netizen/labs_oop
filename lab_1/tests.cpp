#include <iostream>
#include "BitArray.cpp"

using namespace std;

int passed = 0;
int failed = 0;

void check(bool condition, string name) {
    if (condition) {
        passed++;
    } else {
        failed++;
        cout << "FAILED: " << name << endl;
    }
}

void testConstructors() {
    BitArray a;
    check(a.size() == 0 && a.empty(), "default constructor");

    BitArray b(5, 6); // 6 = 110
    check(b.to_string() == "00110", "constructor with value");

    BitArray c(3, 15); // only 3 bits of 1111 fit
    check(c.to_string() == "111", "constructor: extra bits ignored");

    BitArray d(100, 1);
    check(d.size() == 100 && d.count() == 1 && d[0], "constructor: big array");

    bool thrown = false;
    try {
        BitArray e(-1);
    } catch (invalid_argument&) {
        thrown = true;
    }
    check(thrown, "constructor: negative size");

    BitArray copy(b);
    check(copy == b, "copy constructor");
    copy.set(0);
    check(copy != b, "copy is independent");
}

void testAssignAndSwap() {
    BitArray a(4, 5);
    BitArray b(10);
    b = a;
    check(b.to_string() == "0101", "operator=");

    BitArray c(3, 7);
    BitArray d(6, 1);
    c.swap(d);
    check(c.to_string() == "000001" && d.to_string() == "111", "swap");
}

void testResizeClearPushBack() {
    BitArray a(3, 5); // 101
    a.resize(6, true);
    check(a.to_string() == "111101", "resize: grow");
    a.resize(2);
    check(a.to_string() == "01", "resize: shrink");
    a.resize(4);
    check(a.to_string() == "0001", "resize: new bits are 0");

    bool thrown = false;
    try {
        a.resize(-5);
    } catch (invalid_argument&) {
        thrown = true;
    }
    check(thrown, "resize: negative size");

    a.clear();
    check(a.empty(), "clear");

    BitArray b;
    for (int i = 0; i < 100; i++) {
        b.push_back(i % 2 == 0);
    }
    check(b.size() == 100 && b.count() == 50, "push_back");
}

void testBitwise() {
    BitArray a(4, 12); // 1100
    BitArray b(4, 10); // 1010
    check((a & b).to_string() == "1000", "operator&");
    check((a | b).to_string() == "1110", "operator|");
    check((a ^ b).to_string() == "0110", "operator^");

    BitArray other(5);
    int count = 0;
    try { a &= other; } catch (invalid_argument&) { count++; }
    try { a |= other; } catch (invalid_argument&) { count++; }
    try { a ^= other; } catch (invalid_argument&) { count++; }
    check(count == 3, "bitwise: different sizes");
}

void testShifts() {
    BitArray a(6, 11); // 001011
    check((a << 2).to_string() == "101100", "operator<<");
    check((a >> 2).to_string() == "000010", "operator>>");
    check((a << 10).none(), "operator<< by big n");
    check(a.to_string() == "001011", "<< and >> do not change array");

    BitArray big(100, 1);
    big <<= 70;
    check(big[70] && big.count() == 1, "operator<<=");
    big >>= 69;
    check(big[1] && big.count() == 1, "operator>>=");

    int count = 0;
    try { a <<= -1; } catch (invalid_argument&) { count++; }
    try { a >>= -1; } catch (invalid_argument&) { count++; }
    check(count == 2, "shift: negative n");
}

void testSetReset() {
    BitArray a(5);
    a.set(1);
    a.set(3);
    check(a.to_string() == "01010", "set(n)");
    a.set(3, false);
    check(a.to_string() == "00010", "set(n, false)");
    a.set();
    check(a.to_string() == "11111", "set()");
    a.reset(0);
    check(a.to_string() == "11110", "reset(n)");
    a.reset();
    check(a.to_string() == "00000", "reset()");

    int count = 0;
    try { a.set(5); } catch (out_of_range&) { count++; }
    try { a.set(-1); } catch (out_of_range&) { count++; }
    try { a.reset(100); } catch (out_of_range&) { count++; }
    try { a[10]; } catch (out_of_range&) { count++; }
    check(count == 4, "wrong index");
}

void testOther() {
    BitArray zeros(10);
    check(!zeros.any() && zeros.none() && zeros.count() == 0, "all zeros");

    BitArray a(10, 5); // bits 0 and 2
    check(a.any() && !a.none() && a.count() == 2, "any / none / count");
    check(a[0] && !a[1] && a[2], "operator[]");

    BitArray b(5, 6);
    check((~b).to_string() == "11001", "operator~");

    BitArray c(5, 3);
    BitArray d(6, 3);
    check(c != d, "different sizes are not equal");
}

int main() {
    testConstructors();
    testAssignAndSwap();
    testResizeClearPushBack();
    testBitwise();
    testShifts();
    testSetReset();
    testOther();

    cout << "Passed: " << passed << ", failed: " << failed << endl;
    return 0;
}
