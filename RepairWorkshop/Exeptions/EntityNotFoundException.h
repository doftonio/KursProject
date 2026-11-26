#ifndef ENTITYNOTFOUNDEXCEPTION_H
#define ENTITYNOTFOUNDEXCEPTION_H

#include "AppException.h"

class EntityNotFoundException : public AppException {
public:
    explicit EntityNotFoundException(const std::string& message);
};

#endif
