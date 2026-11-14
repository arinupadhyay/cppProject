#include "../include/User.hpp"
#include <chrono>
#include <stdexcept>

User::User(const std::string& id, const std::string& name, const std::string& email, const std::string& role)
    : id(id), name(name), email(email), role(role) {
    if (id.empty()) {
        throw std::invalid_argument("User ID cannot be empty");
    }
    createdAt = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::system_clock::now().time_since_epoch()
    ).count();
}

void User::setName(const std::string& newName) {
    if (newName.empty()) {
        throw std::invalid_argument("User name cannot be empty");
    }
    this->name = newName;
}

void User::setEmail(const std::string& newEmail) {
    if (newEmail.find('@') == std::string::npos) {
        throw std::invalid_argument("Invalid email address format");
    }
    this->email = newEmail;
}

void User::display(std::ostream& os) const {
    os << "[" << role << "] ID: " << id << " | Name: " << name << " | Email: " << email;
}
