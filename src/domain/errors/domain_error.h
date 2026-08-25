#pragma once


namespace will::domain {


enum class DomainErrorCode {
    InvalidArgument,
    NotAuthenticated,
    Unauthorized,
};


struct DomainError {
    DomainErrorCode code = DomainErrorCode::InvalidArgument;
};


} // namespace will::domain
