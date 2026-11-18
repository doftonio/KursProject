#ifndef APPEXCEPTION_H
#define APPEXCEPTION_H

#include <stdexcept>
#include <string>

class AppException : public std::runtime_error {
private:
    int errorCode;

public:
    AppException(const std::string& message, int code = 100);
    int getCode() const noexcept;
};

#endif
