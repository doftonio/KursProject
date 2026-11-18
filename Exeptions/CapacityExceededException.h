#ifndef CAPACITYEXCEEDEDEXCEPTION_H
#define CAPACITYEXCEEDEDEXCEPTION_H

#include "AppException.h"

class CapacityExceededException : public AppException {
public:
    explicit CapacityExceededException(const std::string& message);
};

#endif
