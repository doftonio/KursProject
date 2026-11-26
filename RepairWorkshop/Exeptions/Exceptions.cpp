#include "AppException.h"
#include "InvalidInputDataException.h"
#include "DuplicateEntityException.h"
#include "EntityNotFoundException.h"
#include "CapacityExceededException.h"
#include "InsufficientStockException.h"
#include "FileStorageException.h"

AppException::AppException(const std::string& message, int code)
    : std::runtime_error(message), errorCode(code) {}

int AppException::getCode() const noexcept {
    return errorCode;
}

InvalidInputDataException::InvalidInputDataException(const std::string& message)
    : AppException(message, 101) {}

DuplicateEntityException::DuplicateEntityException(const std::string& message)
    : AppException(message, 102) {}

EntityNotFoundException::EntityNotFoundException(const std::string& message)
    : AppException(message, 103) {}

CapacityExceededException::CapacityExceededException(const std::string& message)
    : AppException(message, 104) {}

InsufficientStockException::InsufficientStockException(const std::string& message)
    : AppException(message, 105) {}

FileStorageException::FileStorageException(const std::string& message)
    : AppException(message, 106) {}
