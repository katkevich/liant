module;
#include "liant/dependency_macro.hpp"

export module wget:logger;

import std;
import liant;

export namespace wget {
class ILogger {
public:
    virtual ~ILogger() = default;

    template <typename... TArgs>
    void logErr(std::string_view message, TArgs&&... args) const {
        logImpl("err: " + std::vformat(message, std::make_format_args(args...)));
    }

    template <typename... TArgs>
    void logInfo(std::string_view message, TArgs&&... args) const {
        logImpl("info: " + std::vformat(message, std::make_format_args(args...)));
    }

protected:
    virtual void logImpl(std::string_view message) const = 0;
};
LIANT_DEPENDENCY(ILogger, logger)

class ConsoleLogger final : public ILogger {
protected:
    virtual void logImpl(std::string_view message) const override;
};
} // namespace wget