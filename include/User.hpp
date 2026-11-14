#ifndef USER_HPP
#define USER_HPP

#include <string>
#include <iostream>

/**
 * ============================================================================
 * OOP CONCEPT: ENCAPSULATION & BASE CLASS INHERITANCE
 * ============================================================================
 * Abstract/Base representation of a participant in the voting ecosystem.
 * Demonstrates:
 * - Protected data members (accessible to derived classes)
 * - Encapsulation via public getters and setters
 * - Virtual destructor for safe polymorphic deletion
 * - Pure virtual / virtual methods for runtime polymorphism
 */
class User {
protected:
    std::string id;
    std::string name;
    std::string email;
    std::string role;
    uint64_t createdAt;

public:
    User(const std::string& id, const std::string& name, const std::string& email, const std::string& role);
    virtual ~User() = default;

    // Getters (Encapsulation)
    std::string getId() const { return id; }
    std::string getName() const { return name; }
    std::string getEmail() const { return email; }
    std::string getRole() const { return role; }
    uint64_t getCreatedAt() const { return createdAt; }

    // Setters with validation (Encapsulation)
    void setName(const std::string& newName);
    void setEmail(const std::string& newEmail);

    // Polymorphic interface methods
    virtual std::string getRoleDescription() const = 0;
    virtual void display(std::ostream& os = std::cout) const;

    // Operator overloading
    bool operator==(const User& other) const {
        return this->id == other.id;
    }
};

#endif // USER_HPP
