#ifndef IVERIFIABLE_HPP
#define IVERIFIABLE_HPP

#include <string>

/**
 * ============================================================================
 * OOP CONCEPT: ABSTRACTION & INTERFACE DESIGN
 * ============================================================================
 * Defines a contract for entities whose cryptographic validity or state
 * can be independently audited and verified across the blockchain network.
 */
class IVerifiable {
public:
    virtual ~IVerifiable() = default;

    /**
     * Pure virtual function: Verifies integrity and cryptographic validity
     * @return true if entity satisfies all validation rules
     */
    virtual bool verify() const = 0;

    /**
     * Pure virtual function: Detailed verification report
     */
    virtual std::string getVerificationDetails() const = 0;
};

#endif // IVERIFIABLE_HPP
