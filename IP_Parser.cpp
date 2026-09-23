/**Author: Jal Maru
Outside sources: Gemini Flash
Date: 9/23/2026
Usage: parse input strings for a valid IP address following rules outline on Canvas
**/

#include <iostream>
#include <string>
#include <cctype>

/**
 * validates whether a single string meets the rules
 * - length under maxlength
 * - value within valuerange
 * - leading zero "0" is valid, but "01", "005", "00" are not
 *
 * on success, populates parsedVal and returns true.
 */
static bool parseNumericComponent(const std::string& token, size_t start, size_t length,
                                  size_t maxLen, unsigned long long maxValue, int& parsedVal) {
    if (length == 0 || length > maxLen) { //check if the length is correct
        return false;
    }

    // check the leading 0 rule
    if (length > 1 && token[start] == '0') {
        return false;
    }

    // manual digit accumulation using unsigned long long to prevent int overflow
    unsigned long long accumulated = 0;
    for (size_t i = 0; i < length; ++i) {
        char ch = token[start + i];
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            return false;
        }
        accumulated = accumulated * 10 + (ch - '0'); //this will accumulate by moving left each new addition
    }

    if (accumulated > maxValue) { //check length again
        return false;
    }

    parsedVal = static_cast<int>(accumulated);  //return true and populate this parsedVal, used later
    return true;
}

/**
 * validates a candidate
 * parses the 4 octets and optional port
 */
static bool validateCandidateToken(const std::string& token,
                                    unsigned long& outAddress,
                                    int& outPort,
                                    std::string& formattedIP) {
    size_t len = token.length();
    size_t pos = 0;
    int octets[4] = {0, 0, 0, 0};

    // parse 4 octets separated by single .
    for (int i = 0; i < 4; ++i) {
        size_t digitStart = pos;
        size_t digitLen = 0;

        while (pos < len && std::isdigit(static_cast<unsigned char>(token[pos]))) {
            pos++;
            digitLen++;
        }

        // octets must have 1-3 digits and value 0-255
        int octetVal = 0;
        if (!parseNumericComponent(token, digitStart, digitLen, 3, 255, octetVal)) { //use the above function to parse each octet
            return false;
        }
        octets[i] = octetVal;

        if (i < 3) {
            //must be followed by exactly one .
            if (pos >= len || token[pos] != '.') {
                return false;
            }
            pos++; // consume .
        }
    }

    // parse for the optional port if a colon follows octet 4
    int portVal = -1;
    if (pos < len) {
        if (token[pos] != ':') {
            return false; // trailing garbage like extra dots or nums
        }
        pos++; //consume :

        size_t portStart = pos;
        size_t portLen = 0;

        while (pos < len && std::isdigit(static_cast<unsigned char>(token[pos]))) {
            pos++;
            portLen++;
        }

        // check if it has 1-5 digits and val 0-65535
        if (!parseNumericComponent(token, portStart, portLen, 5, 65535, portVal)) {
            return false;
        }
    }

    // make sure entire token was consumed
    if (pos != len) {
        return false;
    }

    // build the 32-bit unsigned long decimal value
    outAddress = (static_cast<unsigned long>(octets[0]) << 24) |
                 (static_cast<unsigned long>(octets[1]) << 16) |
                 (static_cast<unsigned long>(octets[2]) << 8)  |
                  static_cast<unsigned long>(octets[3]);

    outPort = portVal;

    //build the formatted string A.B.C.D
    formattedIP = std::to_string(octets[0]) + "." +
                  std::to_string(octets[1]) + "." +
                  std::to_string(octets[2]) + "." +
                  std::to_string(octets[3]);

    return true;
}

/**
 * extracts a single valid address embedded anywhere in str
 */
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort) {
    //default outputs to mark fails
    outAddress = 0;
    outPort = -1;

    size_t i = 0;
    size_t n = str.length();

    int validMatchesCount = 0;
    unsigned long tempAddress = 0;
    int tempPort = -1;

    while (i < n) {
        //find continuous section of allowed characters
        if (std::isdigit(static_cast<unsigned char>(str[i])) || str[i] == '.' || str[i] == ':') {
            size_t start = i;
            while (i < n && (std::isdigit(static_cast<unsigned char>(str[i])) || str[i] == '.' || str[i] == ':')) {
                i++;
            }

            std::string candidate = str.substr(start, i - start);
            std::string formattedIP;

            unsigned long candAddress = 0;
            int candPort = -1;

            if (validateCandidateToken(candidate, candAddress, candPort, formattedIP)) {
                validMatchesCount++;
                tempAddress = candAddress;
                tempPort = candPort;
            }
        } else {
            i++;
        }
    }

    //1 valid add. must be found in string
    if (validMatchesCount == 1) {
        outAddress = tempAddress;
        outPort = tempPort;
        return true;
    }

    return false;
}


//just creating the terminal interaction part
int main() {
    std::string input;

    while (true) {
        std::cout << "Enter a string (or 'END' to quit): ";
        if (!std::getline(std::cin, input)) {
            break;
        }

        if (input == "END") {
            std::cout << "Program terminated." << std::endl;
            break;
        }

        unsigned long address = 0;
        int port = -1;

        if (extractIPv4(input, address, port)) {
            // format manually from te outAddress bits
            unsigned long a = (address >> 24) & 0xFF;
            unsigned long b = (address >> 16) & 0xFF;
            unsigned long c = (address >> 8) & 0xFF;
            unsigned long d = address & 0xFF;

            std::string portStr = (port == -1) ? "none" : std::to_string(port);

            std::cout << "Extracted IPv4 address: "
                      << a << "." << b << "." << c << "." << d
                      << " (decimal value: " << address
                      << ", port: " << portStr << ")" << std::endl;
        } else {
            std::cout << "Invalid input: no valid IPv4 address found" << std::endl;
        }
    }

    return 0;
}
