#pragma once
#include <string>
#include <algorithm>
#include <vector>

class String : public std::string {
public:
    String() : std::string() {}
    String(const char* s) : std::string(s ? s : "") {}
    String(const std::string& s) : std::string(s) {}
    String(int n) : std::string(std::to_string(n)) {}
    String(long n) : std::string(std::to_string(n)) {}
    String(unsigned long n) : std::string(std::to_string(n)) {}
    String(char c) : std::string(1, c) {}

    int toInt() const { try { return std::stoi(*this); } catch(...) { return 0; } }
    
    void replace(const String& from, const String& to) {
        if(from.empty()) return;
        size_t start_pos = 0;
        while((start_pos = this->find(from, start_pos)) != std::string::npos) {
            std::string::replace(start_pos, from.length(), to);
            start_pos += to.length();
        }
    }
    
    String substring(int start, int end) const { 
        if (start >= length()) return "";
        if (end > length()) end = length();
        return this->substr(start, end - start); 
    }
    String substring(int start) const { 
        if (start >= length()) return "";
        return this->substr(start); 
    }
    
    int indexOf(char c) const { return (int)this->find(c); }
    int indexOf(const String& s) const { return (int)this->find(s); }
    int lastIndexOf(char c) const { return (int)this->rfind(c); }
    char charAt(int index) const { return (*this)[index]; }
    
    void toUpperCase() { 
        for(auto& c : *this) c = toupper(c); 
    }
};

inline std::ostream& operator<<(std::ostream& os, const String& s) {
    os << static_cast<const std::string&>(s);
    return os;
}
