#ifndef FILESTORAGEEXCEPTION_H
#define FILESTORAGEEXCEPTION_H

#include "AppException.h"

class FileStorageException : public AppException {
public:
    explicit FileStorageException(const std::string& message);
};

#endif
