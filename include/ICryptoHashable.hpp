#ifndef ICRYPTOHASHABLE_HPP
#define ICRYPTOHASHABLE_HPP

#include <string>

/**
 * ============================================================================
 * OOP CONCEPT: ABSTRACTION (Pure Abstract Interface)
 * ============================================================================
 * Defines a contract for any entity in the blockchain that can be hashed.
 * Classes implementing this interface must provide their own hashing logic.
 */
class ICryptoHashable {
public:
    virtual ~ICryptoHashable() = default;

    /**
     * Pure virtual function: Computes deterministic SHA-256 hash of object state
     */
    virtual std::string computeHash() const = 0;
};

#endif // ICRYPTOHASHABLE_HPP
