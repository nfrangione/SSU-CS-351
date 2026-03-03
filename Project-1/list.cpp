
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
    Bytes bytes;

    Node(Size n) : bytes(n) {
        std::iota(std::begin(bytes), std::end(bytes), 1);
    }

    Hash hash(void) {
        const Hash multiplier = 2654435789;
        Hash hashValue = 104395301;
        
        for (auto byte: bytes) {
            hashValue += (multiplier * byte) ^ (hashValue >> 23);
        }

        return hashValue;
    }

    friend std::ostream& operator << (std::ostream& os, const Node& node) {
        os << &node << " " << node.bytes.size() << ": ";
        for (auto byte : node.bytes) {
            os << (int) byte << " ";
        }
        
        return os;
    }
};

using Nodes = std::list<Node>;

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

    Nodes nodes;
    for (Size i = 0; i < numBlocks; ++i) {
        Size numBytes = getNumBytesForBlock();
        nodes.push_back(Node(numBytes));
    }

    Hash hash = 0;

    for (auto node = std::begin(nodes); node != std::end(nodes); ++node) {
        hash += node->hash();
    }

    std::cout << "list length = " << numBlocks << "  hash = " << hash << std::endl;
}
