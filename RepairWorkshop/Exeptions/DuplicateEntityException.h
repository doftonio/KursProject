#ifndef DUPLICATEENTITYEXCEPTION_H
#define DUPLICATEENTITYEXCEPTION_H

#include "AppException.h"

class DuplicateEntityException : public AppException {
public:
    explicit DuplicateEntityException(const std::string& message);
};

#endif
