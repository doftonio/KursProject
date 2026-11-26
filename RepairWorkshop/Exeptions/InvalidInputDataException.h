#ifndef INVALIDINPUTDATAEXCEPTION_H
#define INVALIDINPUTDATAEXCEPTION_H

#include "AppException.h"

class InvalidInputDataException : public AppException {
public:
    explicit InvalidInputDataException(const std::string& message);
};

#endif
