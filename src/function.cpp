#include "function.hh"
#include "builtin_function.hh"

Functions::Functions(std::shared_ptr<TypeSystem> typeSystem)
    : typeSystem_(typeSystem)
    , scopeManager_()
    , currentScopeId_(0)
    , variable(typeSystem)
{
    BuiltinFunctions<Functions>::registerWith(*this, typeSystem_);
}
