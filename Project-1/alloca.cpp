
#include <alloca.h>
#include <cstdlib>
#include <iostream>
#include <numeric>

using Hash = unsigned long long;
using Size = unsigned int;
using Byte = unsigned char;

struct Node {
    Node* next;
    Size  numBytes;
    Byte* bytes;

    Node(Size n) : next(nullptr), numBytes(n) {
        bytes = reinterpret_cast<Byte*>(&numBytes + sizeof(Size));

        std::iota(bytes, bytes + numBytes, 1);
    }

    Hash hash(void) {
        const Hash multiplier = 2654435789;
        Hash hashValue = 104395301;
        
        for (Size i = 0; i < numBytes; ++i) {
            hashValue += (multiplier * bytes[i]) ^ (hashValue >> 23);
        }

        return hashValue;
    }

    friend std::ostream& operator << (std::ostream& os, const Node& node) {
        os << node.next << " " << node.numBytes << ": ";
        for (Size i = 0; i < node.numBytes; ++i) {
            os << (int) node.bytes[i] << " ";
        }
        
        return os;
    }
};

Size getNumBytesForBlock() {
    const Size MinBytes = MIN_BYTES;
    const Size MaxBytes = MAX_BYTES;

    return (rand() % MaxBytes) + MinBytes;
}

Hash process(Size numBlocks, Node* head, Node* tail) {

    if (numBlocks == 0) {
        Hash hash = 0;

        for (Node* node = head; node != nullptr; node = node->next) {
            hash += node->hash();
        }

        return hash;
    }
    
    Size numBytes = getNumBytesForBlock();
    void* memory = alloca(sizeof(Node) + numBytes);
    Node* node = new (memory) Node(numBytes);

    if (head == nullptr) {
        head = tail = node;
    }

    tail->next = node;
    tail = node;

    return process(numBlocks - 1, head, tail);
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <Number of blocks>" << std::endl;
        return EXIT_FAILURE;
    }

    Size numBlocks = std::stol(argv[1]);

    Node* head = nullptr;
    Node* tail = nullptr;
    Hash hash = process(numBlocks, head, tail);

    std::cout << "list length = " << numBlocks << "  hash = " << hash << std::endl;
}