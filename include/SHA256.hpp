#ifndef SHA256_HPP
#define SHA256_HPP

#include <string>
#include <vector>
#include <cstdint>

/**
 * ============================================================================
 * Pure C++ FIPS 180-4 SHA-256 Implementation
 * Self-contained: No external cryptographic libraries required.
 * ============================================================================
 */
class SHA256 {
public:
    SHA256();
    void update(const uint8_t* data, size_t length);
    void update(const std::string& data);
    std::string digest();

    /**
     * Static convenience function to hash a string directly
     */
    static std::string hash(const std::string& input);

private:
    uint32_t state[8];
    uint64_t bitLength;
    uint8_t buffer[64];
    size_t bufferLength;

    void transform(const uint8_t* chunk);
    void pad();
    void revert(uint8_t* hashOut);

    static uint32_t rotr(uint32_t x, uint32_t n);
    static uint32_t choose(uint32_t e, uint32_t f, uint32_t g);
    static uint32_t majority(uint32_t a, uint32_t b, uint32_t c);
    static uint32_t sig0(uint32_t x);
    static uint32_t sig1(uint32_t x);
    static uint32_t theta0(uint32_t x);
    static uint32_t theta1(uint32_t x);
};

#endif // SHA256_HPP
