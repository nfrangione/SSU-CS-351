
#include <cstdlib>
#include <iostream>
#include <numeric>
#include <vector>

using namespace std;

using Hash = unsigned long long;
using Size = unsigned int;
using Byte = unsigned char;
using Bytes = std::vector<Byte>;

struct Node {
    Node* next;
    Bytes bytes;

    Node(Size n) : next(nullptr), bytes(n)
        { std::iota(std::begin(bytes), std::end(bytes), 1); }

    Hash hash(void) {
        const Hash multiplier = 2654435789;
        Hash hashValue = 104395301;
        
        for (auto byte: bytes) {
            hashValue += (multiplier * byte) ^ (hashValue >> 23);
        }

        return hashValue;
    }

    friend std::ostream& operator << (std::ostream& os, const Node& node) {
        os << node.next << " " << node.bytes.size() << ": ";
        for (auto byte : node.bytes) {
            os << (int) byte << " ";
        }
        
        return os;
    }
};

Size getNumBytesForBlock() {
    const Size MinBytes = MIN_BYTES;
    const Size MaxBytes = MAX_BYTES;

    return (rand() % MaxBytes) + MinBytes;
}

int main(int argc, char* argv[]) {

    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <Number of blocks>" << std::endl;
        return EXIT_FAILURE;
    }

    Size numBlocks = std::stol(argv[1]);

    Node* head = nullptr;
    Node* tail = nullptr;
    for (Size i = 0; i < numBlocks; ++i) {
        Size numBytes = getNumBytesForBlock();
        Node* node = new Node(numBytes);

        if (node == nullptr) {
            std::cerr << "Node allocation failed\n";
            return EXIT_FAILURE;
        }
        
        if (head == nullptr) {
            head = node;
            tail = head;
        }

        tail->next = node;
        tail = node;

    }

    Hash hash = 0;

    for (Node* node = head; node != nullptr; node = node->next) {
        hash += node->hash();
    }

    std::cout << "list length = " << numBlocks << "  hash = "<< hash << std::endl;

    for (Node* node = head; node != nullptr; ) {
        Node* tmp = node;
        node = node->next;
        delete tmp;
    }
}