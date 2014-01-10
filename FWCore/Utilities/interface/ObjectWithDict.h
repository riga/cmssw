#ifndef FWCore_Utilities_ObjectWithDict_h
#define FWCore_Utilities_ObjectWithDict_h

/*----------------------------------------------------------------------

ObjectWithDict:  A holder for an object and its type information.

----------------------------------------------------------------------*/

#include "TInterpreter.h"

#include <string>
#include <typeinfo>

namespace edm {

class TypeWithDict;

class ObjectWithDict {
private:
  TType* type_;
  void* address_;
public:
  static ObjectWithDict byType(TypeWithDict const&);
public:
  ObjectWithDict();
  explicit ObjectWithDict(TypeWithDict const&, void* address);
  explicit ObjectWithDict(std::type_info const&, void* address);
  explicit operator bool() const;
  void* address() const;
  TypeWithDict typeOf() const;
  TypeWithDict dynamicType() const;
  ObjectWithDict get(std::string const& memberName) const;
  //ObjectWithDict construct() const;
  void destruct(bool dealloc) const;
  template<typename T>
  T
  objectCast()
  {
    return *reinterpret_cast<T*>(address_);
  }
};

} // namespace edm

#include "FWCore/Utilities/interface/TypeWithDict.h"

#endif // FWCore_Utilities_ObjectWithDict_h
