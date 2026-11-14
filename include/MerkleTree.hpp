#ifndef MERKLETREE_HPP
#define MERKLETREE_HPP

#include "SHA256.hpp"
#include <string>
#include <vector>
#include <memory>
#include <iostream>

/**
 * ============================================================================
 * OOP CONCEPT: TEMPLATES & GENERIC PROGRAMMING (Compile-time Polymorphism)
 * ============================================================================
 * Templated Merkle Tree that can construct cryptographic audit trees over any
 * list of hashable elements (transactions, votes, certificates, or strings).
 */
template <typename T>
class MerkleTree {
public:
    struct Node {
        std::string hash;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;

        Node(const std::string& h) : hash(h), left(nullptr), right(nullptr) {}
    };

    struct AuditStep {
        std::string position; // "left" or "right"
        std::string hash;
    };

private:
    std::shared_ptr<Node> root;
    std::vector<std::string> leafHashes;

    std::shared_ptr<Node> buildTree(const std::vector<std::shared_ptr<Node>>& nodes) {
        if (nodes.empty()) {
            return std::make_shared<Node>(SHA256::hash("EMPTY_TREE"));
        }
        if (nodes.size() == 1) {
            return nodes[0];
        }

        std::vector<std::shared_ptr<Node>> parentLevel;
        for (size_t i = 0; i < nodes.size(); i += 2) {
            std::shared_ptr<Node> left = nodes[i];
            std::shared_ptr<Node> right = (i + 1 < nodes.size()) ? nodes[i + 1] : left;

            std::string combinedHash = SHA256::hash(left->hash + right->hash);
            auto parent = std::make_shared<Node>(combinedHash);
            parent->left = left;
            parent->right = right;
            parentLevel.push_back(parent);
        }

        return buildTree(parentLevel);
    }

public:
    MerkleTree() : root(nullptr) {}

    explicit MerkleTree(const std::vector<std::string>& hashes) {
        setLeaves(hashes);
    }

    void setLeaves(const std::vector<std::string>& hashes) {
        leafHashes = hashes;
        if (hashes.empty()) {
            root = std::make_shared<Node>(SHA256::hash("EMPTY_TREE"));
            return;
        }

        std::vector<std::shared_ptr<Node>> leaves;
        for (const auto& h : hashes) {
            leaves.push_back(std::make_shared<Node>(h));
        }
        root = buildTree(leaves);
    }

    std::string getRootHash() const {
        return root ? root->hash : SHA256::hash("EMPTY_TREE");
    }

    /**
     * Generates a cryptographic audit proof for a leaf at targetIndex
     */
    std::vector<AuditStep> generateProof(size_t targetIndex) const {
        std::vector<AuditStep> proof;
        if (targetIndex >= leafHashes.size()) return proof;

        std::vector<std::string> currentLevel = leafHashes;
        size_t index = targetIndex;

        while (currentLevel.size() > 1) {
            bool isRight = (index % 2 == 1);
            size_t pairIndex = isRight ? (index - 1) : (index + 1);

            AuditStep step;
            step.position = isRight ? "left" : "right";

            if (pairIndex < currentLevel.size()) {
                step.hash = currentLevel[pairIndex];
            } else {
                step.hash = currentLevel[index];
            }
            proof.push_back(step);

            std::vector<std::string> nextLevel;
            for (size_t i = 0; i < currentLevel.size(); i += 2) {
                std::string left = currentLevel[i];
                std::string right = (i + 1 < currentLevel.size()) ? currentLevel[i + 1] : left;
                nextLevel.push_back(SHA256::hash(left + right));
            }
            currentLevel = nextLevel;
            index /= 2;
        }

        return proof;
    }

    /**
     * Static verification of a Merkle proof
     */
    static bool verifyProof(const std::string& leafHash,
                            const std::vector<AuditStep>& proof,
                            const std::string& expectedRoot) {
        std::string current = leafHash;
        for (const auto& step : proof) {
            if (step.position == "left") {
                current = SHA256::hash(step.hash + current);
            } else {
                current = SHA256::hash(current + step.hash);
            }
        }
        return current == expectedRoot;
    }
};

#endif // MERKLETREE_HPP
