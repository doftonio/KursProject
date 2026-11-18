#ifndef INSUFFICIENTSTOCKEXCEPTION_H
#define INSUFFICIENTSTOCKEXCEPTION_H

#include "AppException.h"

class InsufficientStockException : public AppException {
public:
    explicit InsufficientStockException(const std::string& message);
};

#endif
