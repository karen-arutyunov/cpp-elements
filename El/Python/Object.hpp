/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/Python/Object.hpp
 * @author Karen Arutyunov
 * $id:$
 */

#ifndef _ELEMENTS_EL_PYTHON_OBJECT_HPP_
#define _ELEMENTS_EL_PYTHON_OBJECT_HPP_

#include <string.h>

#include <map>
#include <sstream>
#include <string>
#include <vector>

#include <Python.h>
#include <structmember.h>

#include <El/Exception.hpp>
#include <El/String/Manip.hpp>
#include <El/BinaryStream.hpp>

#include <El/Python/Exception.hpp>
#include <El/Python/RefCount.hpp>

#if PY_VERSION_HEX < 0x02050000 && !defined(PY_SSIZE_T_MIN)

  typedef int Py_ssize_t;
# define PY_SSIZE_T_MAX INT_MAX
# define PY_SSIZE_T_MIN INT_MIN

#endif

namespace El
{
  namespace Python
  {
    class Module;

    bool el_based_object(PyObject* obj) throw();

    void handle_error(const char* context = 0,
                      const char* suffix = 0)
      throw(Exception, El::Exception);

    void report_error(PyObject* type,
                      const char* description,
                      const char* context = 0,
                      const char* suffix = 0)
      throw(Exception, El::Exception);

    void set_error(El::Exception& e) throw();

    void set_runtime_error(const char* error) throw();      
    void set_index_error(const char* error) throw();      
    void set_type_error(const char* error) throw();      
    void set_key_error(const char* error) throw();

    std::ostream& print(std::ostream& ostr,
                        const PyObject* obj,
                        const char* text_on_failure = 0)
      throw(El::Exception);    
    
    struct ObjectImpl : public PyObject
    {
      ObjectImpl(PyTypeObject* type) throw();
      ObjectImpl(const ObjectImpl& val) throw();
      
      virtual ~ObjectImpl() throw() {}

      enum CMP_RESULT
      {
        CR_FALSE,
        CR_TRUE,
        CR_NOT_IMPL
      };

      virtual CMP_RESULT eq(ObjectImpl* ob) throw(Exception, El::Exception);
      virtual CMP_RESULT ne(ObjectImpl* ob) throw(Exception, El::Exception);
      virtual CMP_RESULT lt(ObjectImpl* ob) throw(Exception, El::Exception);
      virtual CMP_RESULT gt(ObjectImpl* ob) throw(Exception, El::Exception);
      virtual CMP_RESULT le(ObjectImpl* ob) throw(Exception, El::Exception);
      virtual CMP_RESULT ge(ObjectImpl* ob) throw(Exception, El::Exception);

      virtual PyObject* str() throw(Exception, El::Exception);

      virtual void constant(bool val) throw();
      
      virtual void write(El::BinaryOutStream& bstr) const
        throw(NotImplemented, El::Exception);
      
      virtual void read(El::BinaryInStream& bstr)
        throw(NotImplemented, El::Exception);
      
      bool is_constant() const throw() { return constant_; }
      bool is_constant_ptr(ObjectImpl** ptr) const throw();

      ObjectImpl& operator=(const ObjectImpl&) throw();

      template <typename VAR>
      void init(VAR& member, typename VAR::Type* obj, bool make_const = false)
        throw(El::Exception);
      
    protected:
      typedef std::vector<ObjectImpl**> SubObjArray;
      SubObjArray sub_objects_;
      
      bool constant_;
    };

    class ObjectType : public PyTypeObject
    {
    public:
      ObjectType(const char* type_name,
                 const char* type_doc,
                 const char* base)
        throw(Exception, El::Exception);
      
      virtual ~ObjectType() throw();

      static void init() throw(Exception, El::Exception);
      static void terminate() throw(El::Exception);
      static ObjectType* find(const char* name) throw();
      
      virtual void initialized() throw(Exception, El::Exception) {}
      
      void add_to_module(PyObject* module) throw(Exception, El::Exception);

    protected:

      friend class El::Python::Module;

      typedef std::map<std::string, ObjectType*> ObjectTypeMap;

      static ObjectTypeMap& registry() throw(El::Exception);

      void clear() throw();

      void add_member(getter get_func,
                      setter set_func,
                      const char* name,
                      const char* doc)
        throw(El::Exception);
      
      void add_method(PyCFunction func,
                      int flags,
                      const char* name,
                      const char* doc)
        throw(El::Exception);

      virtual void ready() throw(Exception, El::Exception);

    private:
      static PyObject* object_str(PyObject* obj) throw();

    private:      
      unsigned long method_count_;
      unsigned long member_count_;
      std::string base_type_;
      
      static bool initialized_;
    };
    
    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    class ObjectTypeImpl : public ObjectType
    {
    public:

      ObjectTypeImpl(const char* type_name,
                     const char* type_doc,
                     const char* base = 0)
        throw(Exception, El::Exception);
      
      ~ObjectTypeImpl() throw();

      static OBJECT_CLASS* create(PyObject* args = 0, PyObject* kwds = 0)
        throw(El::Exception);

      static OBJECT_CLASS* down_cast(PyObject* object,
                                     bool add_ref = false,
                                     bool exception = true)
        throw(Exception, El::Exception);

      static bool check_type(PyObject* ob) throw();
      static bool is_type(PyObject* ob) throw();

    public:

      typedef OBJECT_CLASS ObjectClass;
      typedef TYPE_CLASS TypeClass;
      
      typedef const char* (*Str)();
      typedef PyObject* (TypeClass:: *GetObj)();
      
      typedef PyObject* (ObjectClass::* NoArgsMethod)();
      typedef PyObject* (ObjectClass::* VarArgsMethod)(PyObject* args);
      
      typedef PyObject* (ObjectClass::* KwdsMethod)(PyObject* args,
                                                    PyObject* kwds);
      
      typedef PyObject* (ObjectClass::* GetterMethod)();
      typedef void (ObjectClass::* SetterMethod)(PyObject* value);

      static PyObject* disp_noargs(OBJECT_CLASS* self, NoArgsMethod method)
        throw();
      
      static PyObject* disp_varargs(OBJECT_CLASS* self,
                                    VarArgsMethod method,
                                    PyObject* args) throw();
      
      static PyObject* disp_kwds(OBJECT_CLASS* self,
                                 KwdsMethod method,
                                 PyObject* args,
                                 PyObject* kwds) throw();
      
      static PyObject* disp_getter(OBJECT_CLASS* self, GetterMethod method)
        throw();
      
      static int disp_setter(OBJECT_CLASS* self,
                             SetterMethod method,
                             PyObject *value) throw();

      template <typename TYPE_CLASS_,
                typename FUNC_TYPE,
                FUNC_TYPE func,
                int flags,
                Str name,
                Str doc>
      struct MethodUpdater
      {
        MethodUpdater() throw()
        {
          TYPE_CLASS_::instance.add_method(reinterpret_cast<PyCFunction>(func),
                                           flags,
                                           (*name)(),
                                           (*doc)());
        }
      };

      template <typename TYPE_CLASS_,
                getter get_func,
                setter set_func,
                Str name,
                Str doc,
                bool constant = false>
      struct MemberUpdater
      {
        MemberUpdater() throw()
        {
          TYPE_CLASS_::instance.add_member(get_func,
                                           constant ? (setter)0 : set_func,
                                           (*name)(),
                                           (*doc)());
        }
      };      

      template <typename TYPE_CLASS_, GetObj object, Str name>
      struct StaticMemberUpdater
      {
        StaticMemberUpdater() throw()
        {
          TYPE_CLASS_::instance.add_static_member((*name)(), object);
        }
      };

    protected:
      virtual void ready() throw(Exception, El::Exception);
      
    private:
      
      void add_static_member(const char* name, GetObj get_obj)
        throw(El::Exception);
      
      static void type_dealloc(PyObject* self) throw();
      
      static PyObject* type_new(PyTypeObject *type,
                                PyObject *args,
                                PyObject *kwds) throw();      

      static PyObject* richcompare(PyObject* o1, PyObject* o2, int op) throw();
      
    private:
      typedef std::map<std::string, GetObj> StaticMemberMap;
      StaticMemberMap static_members_;
    };

  }
}

#define PY_TYPE_STATIC_MEMBER(member, py_name) \
  PyObject* member##_object__() \
  { \
    return El::Python::add_ref(TypeClass::instance.member.in()); \
  }\
  static const char* member##_name__() \
  { \
    return py_name; \
  }\
  StaticMemberUpdater<TypeClass, \
                      &TypeClass::member##_object__, \
                      member##_name__> \
  member##_static_upd__;

#define PY_TYPE_METHOD_DEF_(func_type, method, flags, py_name, py_doc)  \
  static const char* method##_name__() { return py_name; } \
  static const char* method##_doc__() { return py_doc; } \
  MethodUpdater<TypeClass, func_type, method##_disp__, flags, \
                method##_name__, method##_doc__> \
  method##_upd__;

#define PY_TYPE_MEMBER_DEF_(member, py_name, py_doc)  \
  static const char* member##_name__() { return py_name; } \
  static const char* member##_doc__() { return py_doc; } \
  MemberUpdater<TypeClass, get_##member##__, set_##member##__, \
                member##_name__, member##_doc__>             \
  member##_upd__;

#define PY_TYPE_MEMBER_CONSTANTNESS_CHECK(obj, py_name) \
  if(obj->is_constant()) \
  {                                                             \
    PyErr_SetString(PyExc_TypeError,                                    \
                    "attribute " #py_name " of the object is not writable"); \
    return -1;                                                          \
  }                                                                     \

#define PY_TYPE_OBJECT_MEMBER_CONSTANTNESS_CHECK(obj, member, py_name) \
  if(obj->is_constant() || obj->is_constant_ptr(reinterpret_cast<ObjectImpl**>(obj->member.out()))) \
  {                                                             \
    PyErr_SetString(PyExc_TypeError,                                    \
                    "attribute " #py_name " of the object is not writable"); \
    return -1;                                                          \
  }                                                                     \

#define PY_TYPE_METHOD_NOARGS(method, py_name, py_doc) \
  static PyObject* method##_disp__(PyObject* self)          \
  { \
    return disp_noargs(static_cast<ObjectClass*>(self), &ObjectClass::method); \
  } \
  PY_TYPE_METHOD_DEF_(PyNoArgsFunction, method, METH_NOARGS, py_name, py_doc)

#define PY_TYPE_METHOD_VARARGS(method, py_name, py_doc) \
  static PyObject* method##_disp__(PyObject* self, PyObject* args)           \
  { \
    return disp_varargs(static_cast<ObjectClass*>(self), &ObjectClass::method, \
                        args); \
  } \
  PY_TYPE_METHOD_DEF_(PyCFunction, method, METH_VARARGS, py_name, py_doc)

#define PY_TYPE_METHOD_KWDS(method, py_name, py_doc) \
  static PyObject* method##_disp__(PyObject* self, PyObject* args, \
                                   PyObject* kwds)                 \
  { \
    return disp_kwds(static_cast<ObjectClass*>(self), &ObjectClass::method, \
                     args, kwds); \
  } \
  PY_TYPE_METHOD_DEF_(PyCFunctionWithKeywords, method, \
                      METH_VARARGS | METH_KEYWORDS, py_name, py_doc)

#define PY_TYPE_MEMBER(get_method, set_method, py_name, py_doc) \
  static PyObject* get_method##_disp__(PyObject* self, void *closure) \
  { \
    return disp_getter(static_cast<ObjectClass*>(self), \
                       &ObjectClass::get_method);       \
  } \
  static int set_method##_disp__(PyObject* self, PyObject *value, void *closure) \
  { \
    return disp_setter(static_cast<ObjectClass*>(self), \
                       &ObjectClass::set_method, value); \
  } \
  static const char* get_method##_name__() { return py_name; } \
  static const char* get_method##_doc__() { return py_doc; } \
  MemberUpdater<TypeClass, get_method##_disp__, set_method##_disp__, \
                get_method##_name__, get_method##_doc__>             \
  get_method##_upd__;

#define PY_TYPE_CONST_MEMBER(get_method, py_name, py_doc) \
  static PyObject* get_method##_disp__(PyObject* self, void *closure) \
  { \
    return disp_getter(static_cast<ObjectClass*>(self), \
                       &ObjectClass::get_method);       \
  } \
  static int dull_method##_disp__(PyObject* self, PyObject *value, void *closure) \
  { \
    return 0; \
  } \
  static const char* get_method##_name__() { return py_name; } \
  static const char* get_method##_doc__() { return py_doc; } \
  MemberUpdater<TypeClass, get_method##_disp__, dull_method##_disp__, \
                get_method##_name__, get_method##_doc__, true>             \
  get_method##_upd__;

#define PY_TYPE_MEMBER_BOOL(member, py_name, py_doc)  \
  static PyObject* get_##member##__(PyObject* self, void *closure) \
  { \
    PyObject* res = static_cast<ObjectClass*>(self)->member ? \
      Py_True : Py_False; \
    Py_XINCREF(res); \
    return res;    \
  } \
  static int set_##member##__(PyObject* self, PyObject *value, void *closure) \
  { \
    ObjectClass* m = static_cast<ObjectClass*>(self);   \
    PY_TYPE_MEMBER_CONSTANTNESS_CHECK(m, py_name)        \
    if(value && !PyBool_Check(value)) \
    { \
      PyErr_SetString(PyExc_TypeError, \
                      "Value for " #py_name " should be of bool type"); \
      return -1; \
    } \
    m->member = value == Py_True;      \
    return 0; \
  } \
  PY_TYPE_MEMBER_DEF_(member, py_name, py_doc)

#define PY_TYPE_MEMBER_INT(member, py_name, py_doc)  \
  static PyObject* get_##member##__(PyObject* self, void *closure) \
  { \
    return PyInt_FromLong(static_cast<ObjectClass*>(self)->member); \
  } \
  static int set_##member##__(PyObject* self, PyObject *value, void *closure) \
  { \
    ObjectClass* m = static_cast<ObjectClass*>(self);   \
    PY_TYPE_MEMBER_CONSTANTNESS_CHECK(m, py_name) \
    if(value && !PyInt_Check(value)) \
    { \
      PyErr_SetString(PyExc_TypeError, \
                      "Value for " #py_name " should be of int type"); \
      return -1; \
    } \
    m->member = value ? PyInt_AsLong(value) : 0;      \
    return 0; \
  } \
  PY_TYPE_MEMBER_DEF_(member, py_name, py_doc)

#define PY_TYPE_MEMBER_LONGLONG(member, py_name, py_doc)  \
  static PyObject* get_##member##__(PyObject* self, void *closure) \
  { \
    return PyLong_FromLongLong(static_cast<ObjectClass*>(self)->member); \
  } \
  static int set_##member##__(PyObject* self, PyObject *value, void *closure) \
  { \
    ObjectClass* m = static_cast<ObjectClass*>(self);   \
    PY_TYPE_MEMBER_CONSTANTNESS_CHECK(m, py_name) \
    if(value && !PyLong_Check(value) && !PyInt_Check(value)) \
    { \
      PyErr_SetString(PyExc_TypeError, \
                      "Value for " #py_name " should be of long type"); \
      return -1; \
    } \
    m->member = value ? (PyLong_Check(value) ? \
                         PyLong_AsLongLong(value) :      \
                         PyInt_AsLong(value)) : 0;      \
    return 0; \
  } \
  PY_TYPE_MEMBER_DEF_(member, py_name, py_doc)

#define PY_TYPE_MEMBER_ULONGLONG(member, py_name, py_doc)  \
  static PyObject* get_##member##__(PyObject* self, void *closure) \
  { \
    return PyLong_FromUnsignedLongLong(static_cast<ObjectClass*>(self)->member); \
  } \
  static int set_##member##__(PyObject* self, PyObject *value, void *closure) \
  { \
    ObjectClass* m = static_cast<ObjectClass*>(self);   \
    PY_TYPE_MEMBER_CONSTANTNESS_CHECK(m, py_name) \
    if(value && !PyLong_Check(value) && !PyInt_Check(value)) \
    { \
      PyErr_SetString(PyExc_TypeError, \
                      "Value for " #py_name " should be of long type"); \
      return -1; \
    } \
    m->member = value ? (PyLong_Check(value) ? \
                         PyLong_AsUnsignedLongLong(value) : \
                         PyInt_AsLong(value)) : 0;         \
    return 0; \
  } \
  PY_TYPE_MEMBER_DEF_(member, py_name, py_doc)

#define PY_TYPE_MEMBER_LONG(member, py_name, py_doc)  \
  static PyObject* get_##member##__(PyObject* self, void *closure) \
  { \
    return PyLong_FromLong(static_cast<ObjectClass*>(self)->member); \
  } \
  static int set_##member##__(PyObject* self, PyObject *value, void *closure) \
  { \
    ObjectClass* m = static_cast<ObjectClass*>(self);   \
    PY_TYPE_MEMBER_CONSTANTNESS_CHECK(m, py_name) \
    if(value && !PyLong_Check(value) && !PyInt_Check(value)) \
    { \
      PyErr_SetString(PyExc_TypeError, \
                      "Value for " #py_name " should be of long type"); \
      return -1; \
    } \
    m->member = value ? (PyLong_Check(value) ? \
                         PyLong_AsLong(value) : \
                         PyInt_AsLong(value)) : 0;         \
    return 0; \
  } \
  PY_TYPE_MEMBER_DEF_(member, py_name, py_doc)

#define PY_TYPE_MEMBER_ULONG(member, py_name, py_doc)  \
  static PyObject* get_##member##__(PyObject* self, void *closure) \
  { \
    return PyLong_FromUnsignedLong(static_cast<ObjectClass*>(self)->member); \
  } \
  static int set_##member##__(PyObject* self, PyObject *value, void *closure) \
  { \
    ObjectClass* m = static_cast<ObjectClass*>(self);   \
    PY_TYPE_MEMBER_CONSTANTNESS_CHECK(m, py_name) \
    if(value && !PyLong_Check(value) && !PyInt_Check(value)) \
    { \
      PyErr_SetString(PyExc_TypeError, \
                      "Value for " #py_name " should be of long type"); \
      return -1; \
    } \
    m->member = value ? (PyLong_Check(value) ? \
                         PyLong_AsUnsignedLong(value) : \
                         PyInt_AsLong(value)) : 0;         \
    return 0; \
  } \
  PY_TYPE_MEMBER_DEF_(member, py_name, py_doc)

#define PY_TYPE_MEMBER_ENUM(member, member_type, member_max, py_name, py_doc) \
  static PyObject* get_##member##__(PyObject* self, void *closure) \
  { \
    return PyLong_FromUnsignedLong(static_cast<ObjectClass*>(self)->member); \
  } \
  static int set_##member##__(PyObject* self, PyObject *value, void *closure) \
  { \
    ObjectClass* m = static_cast<ObjectClass*>(self);   \
    PY_TYPE_MEMBER_CONSTANTNESS_CHECK(m, py_name) \
    if(value && !PyLong_Check(value) && !PyInt_Check(value)) \
    { \
      PyErr_SetString(PyExc_TypeError, \
                      "Value for " #py_name " should be of long type"); \
      return -1; \
    } \
    unsigned long val = value ? (PyLong_Check(value) ? \
                                 PyLong_AsUnsignedLong(value) : \
                                 PyInt_AsLong(value)) : 0;         \
    if(val > member_max) \
    { \
      std::ostringstream ostr; \
      ostr << "Value for " #py_name " should be not greater than " << member_max; \
      PyErr_SetString(PyExc_TypeError, ostr.str().c_str()); \
      return -1; \
    } \
    m->member = (member_type)val; \
    return 0; \
  } \
  PY_TYPE_MEMBER_DEF_(member, py_name, py_doc)

#define PY_TYPE_MEMBER_FLOAT(member, py_name, py_doc)  \
  static PyObject* get_##member##__(PyObject* self, void *closure) \
  { \
    return PyFloat_FromDouble(static_cast<ObjectClass*>(self)->member); \
  } \
  static int set_##member##__(PyObject* self, PyObject *value, void *closure) \
  { \
    ObjectClass* m = static_cast<ObjectClass*>(self);   \
    PY_TYPE_MEMBER_CONSTANTNESS_CHECK(m, py_name) \
    if(value && !PyFloat_Check(value)) \
    { \
      PyErr_SetString(PyExc_TypeError, \
                      "Value for " #py_name " should be of float type"); \
      return -1; \
    } \
    m->member = value ? PyFloat_AsDouble(value) : 0;      \
    return 0; \
  } \
  PY_TYPE_MEMBER_DEF_(member, py_name, py_doc)

#define PY_TYPE_MEMBER_STRING(member, py_name, py_doc, can_clear)  \
  static PyObject* get_##member##__(PyObject* self, void *closure) \
  { \
    return PyString_FromString( \
      static_cast<ObjectClass*>(self)->member.c_str()); \
  } \
  static int set_##member##__(PyObject* self, PyObject *value, void *closure) \
  { \
    ObjectClass* m = static_cast<ObjectClass*>(self);   \
    PY_TYPE_MEMBER_CONSTANTNESS_CHECK(m, py_name) \
    if(value && !PyString_Check(value)) \
    { \
      PyErr_SetString(PyExc_TypeError, \
                      "Value for " #py_name " should be a string"); \
      return -1; \
    } \
    if(value == 0 || *PyString_AsString(value) == '\0') \
    { \
      if(can_clear) \
      { \
        m->member.clear();                      \
        return 0; \
      } \
      PyErr_SetString(PyExc_TypeError, \
                      "Cannot clear the " #py_name " attribute"); \
      return -1; \
    }\
    m->member = PyString_AsString(value); \
    return 0; \
  } \
  PY_TYPE_MEMBER_DEF_(member, py_name, py_doc)

#define PY_TYPE_MEMBER_WSTRING(member, py_name, py_doc, can_clear)  \
  static PyObject* get_##member##__(PyObject* self, void *closure) \
  { \
    std::wstring& member = static_cast<ObjectClass*>(self)->member; \
    return PyUnicode_FromWideChar(member.c_str(), member.length()); \
  } \
  static int set_##member##__(PyObject* self, PyObject *value, void *closure) \
  { \
    ObjectClass* m = static_cast<ObjectClass*>(self);   \
    PY_TYPE_MEMBER_CONSTANTNESS_CHECK(m, py_name) \
    bool is_string = false; \
    if(value && !(is_string = PyString_Check(value)) && \
       !PyUnicode_Check(value))                        \
    { \
      PyErr_SetString(PyExc_TypeError, \
                      "Value for " #py_name " should be a unicode object"); \
      return -1; \
    } \
    if(value == 0 || (is_string ? *PyString_AsString(value) == '\0' :\
                      *PyUnicode_AsUnicode(value) == 0)) \
    { \
      if(can_clear) \
      { \
        m->member.clear();                      \
        return 0; \
      } \
      PyErr_SetString(PyExc_TypeError, \
                      "Cannot clear the " #py_name " attribute"); \
      return -1; \
    }\
    if(is_string)\
    {\
      El::String::Manip::utf8_to_wchar(PyString_AsString(value), m->member); \
    }\
    else                                        \
    {   \
      int size = PyUnicode_GetSize(value); \
      El::ArrayPtr<wchar_t> buff(new wchar_t[size + 1]); \
      if(PyUnicode_AsWideChar((PyUnicodeObject*)value, buff.get(), size + 1) < \
         0) \
      {\
        return -1;\
      }\
      buff[size] = L'\0';\
      m->member = buff.get();\
    }\
    return 0; \
  } \
  PY_TYPE_MEMBER_DEF_(member, py_name, py_doc)

#define PY_TYPE_MEMBER_OBJECT(member, member_type, py_name, py_doc, can_del) \
  static PyObject* get_##member##__(PyObject* self, void *closure) \
  { \
    PyObject* member = static_cast<ObjectClass*>(self)->member.in();    \
    return El::Python::add_ref(member ? member : Py_None);              \
  } \
  static int set_##member##__(PyObject* self, PyObject *value, void *closure) \
  { \
    ObjectClass* m = static_cast<ObjectClass*>(self);   \
    PY_TYPE_OBJECT_MEMBER_CONSTANTNESS_CHECK(m, member, py_name) \
    if(value == 0) \
    { \
      if(can_del) \
      { \
        m->member = 0; \
        return 0; \
      } \
      PyErr_SetString(PyExc_TypeError, \
                      "Cannot delete the " #py_name " attribute"); \
      return -1; \
    }\
    if(!member_type::check_type(value)) \
    { \
      std::ostringstream ostr; \
      ostr << "Value for " #py_name " should be of type " \
           << member_type::instance.tp_name; \
      PyErr_SetString(PyExc_TypeError, ostr.str().c_str());      \
      return -1; \
    } \
    m->member = member_type::down_cast(value, true);  \
    return 0; \
  } \
  PY_TYPE_MEMBER_DEF_(member, py_name, py_doc)

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

namespace El
{
  namespace Python
  {
    //
    // ObjectImpl class
    //
    inline
    ObjectImpl::ObjectImpl(PyTypeObject* type) throw()
        : constant_(false)
    {
      PyObject head = { PyObject_HEAD_INIT(type) };
      *static_cast<PyObject*>(this) = head;
      sub_objects_.reserve(0);
    }

    inline
    ObjectImpl::ObjectImpl(const ObjectImpl& val) throw()
        : constant_(val.constant_)
    {
      PyObject head = { PyObject_HEAD_INIT(val.ob_type) };
      *static_cast<PyObject*>(this) = head;
      sub_objects_.reserve(0);
    }

    inline
    ObjectImpl::CMP_RESULT
    ObjectImpl::lt(ObjectImpl* ob) throw(Exception, El::Exception)
    {
      return CR_NOT_IMPL;
    }

    inline
    ObjectImpl::CMP_RESULT
    ObjectImpl::eq(ObjectImpl* ob) throw(Exception, El::Exception)
    {
      ObjectImpl::CMP_RESULT res = lt(ob);

      if(res == CR_NOT_IMPL)
      {
        return CR_NOT_IMPL;
      }
      
      return res == CR_FALSE && ob->lt(this) == CR_FALSE ? CR_TRUE : CR_FALSE;
    }  

    inline
    ObjectImpl::CMP_RESULT
    ObjectImpl::ge(ObjectImpl* ob) throw(Exception, El::Exception)
    {
      switch(lt(ob))
      {
      case CR_TRUE: return CR_FALSE;
      case CR_FALSE: return CR_TRUE;
      case CR_NOT_IMPL: return CR_NOT_IMPL;
      }

      return CR_NOT_IMPL;
    }

    inline
    ObjectImpl::CMP_RESULT
    ObjectImpl::ne(ObjectImpl* ob) throw(Exception, El::Exception)
    {
      switch(eq(ob))
      {
      case CR_TRUE: return CR_FALSE;
      case CR_FALSE: return CR_TRUE;
      case CR_NOT_IMPL: return CR_NOT_IMPL;
      }

      return CR_NOT_IMPL;
    }  

    inline
    ObjectImpl::CMP_RESULT
    ObjectImpl::le(ObjectImpl* ob) throw(Exception, El::Exception)
    {
      ObjectImpl::CMP_RESULT ltr = lt(ob);
      ObjectImpl::CMP_RESULT eqr = eq(ob);
        
      return ltr == CR_NOT_IMPL || eqr == CR_NOT_IMPL ? CR_NOT_IMPL :
        (ltr == CR_TRUE || eqr == CR_TRUE ? CR_TRUE : CR_FALSE);
    }  

    inline
    ObjectImpl::CMP_RESULT
    ObjectImpl::gt(ObjectImpl* ob) throw(Exception, El::Exception)
    {
      switch(le(ob))
      {
      case CR_TRUE: return CR_FALSE;
      case CR_FALSE: return CR_TRUE;
      case CR_NOT_IMPL: return CR_NOT_IMPL;
      }

      return CR_NOT_IMPL;
    }

    inline
    ObjectImpl&
    ObjectImpl::operator=(const ObjectImpl& val) throw()
    {
      constant_ = val.constant_;
      return *this;
    }

    inline
    void
    ObjectImpl::constant(bool val) throw()
    {
      constant_ = val;
      
      for(SubObjArray::iterator i(sub_objects_.begin()), e(sub_objects_.end());
          i != e; ++i)
     {
        ObjectImpl* p = **i;

        if(el_based_object(p))
        {
          p->constant(val);
        }
      }
    }

    inline
    void
    ObjectImpl::write(El::BinaryOutStream& bstr) const
      throw(NotImplemented, El::Exception)
    {
      std::ostringstream ostr;
      ostr << "El::Python::ObjectImpl::write: not implemented for type "
           << (const char*)ob_type->tp_name;

      throw NotImplemented(ostr.str());
    }

    inline
    void
    ObjectImpl::read(El::BinaryInStream& bstr)
      throw(NotImplemented, El::Exception)
    {
      std::ostringstream ostr;
      ostr << "El::Python::ObjectImpl::read: not implemented for type "
           << (const char*)ob_type->tp_name;

      throw NotImplemented(ostr.str());
    }

    inline
    bool
    ObjectImpl::is_constant_ptr(ObjectImpl** ptr) const throw()
    {
      for(SubObjArray::const_iterator i(sub_objects_.begin()),
            e(sub_objects_.end()); i != e; ++i)
     {
        ObjectImpl** p = *i;

        if(p == ptr)
        {
          return (*p)->is_constant();
        }
      }

      return false;
    }

    template <typename VAR>
    void
    ObjectImpl::init(VAR& member,
                     typename VAR::Type* obj,
                     bool make_const) throw(El::Exception)
    {
      member = obj;

      if(make_const)
      {
        member->constant(true);
      }
      
      sub_objects_.push_back(
        reinterpret_cast<El::Python::ObjectImpl**>(member.out()));
    }
    
    //
    // ObjectTypeImpl class
    //
    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::ObjectTypeImpl(
      const char* type_name,
      const char* type_doc,
      const char* base)
      throw(Exception, El::Exception)
        : ObjectType(type_name, type_doc, base)
    {
      tp_dealloc = type_dealloc;
      tp_new = type_new;

      tp_richcompare = (richcmpfunc)&richcompare;
      tp_flags |= Py_TPFLAGS_HAVE_RICHCOMPARE;
    }
        
    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::~ObjectTypeImpl() throw()
    {
    }
    
    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    void
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::add_static_member(
      const char* name, GetObj get_obj)
      throw(El::Exception)
    { 
      static_members_[name] = get_obj;
    } 

    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    void
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::ready()
      throw(Exception, El::Exception)
    {
      ObjectType::ready();
      
      for(typename StaticMemberMap::iterator
            it = static_members_.begin();
          it != static_members_.end(); it++)
      {
        GetObj getobj = it->second;
        El::Python::Object_var obj = (((TypeClass*)this)->*getobj)();

        if(obj.in() == 0)
        {
          std::ostringstream ostr;
          ostr << "El::Python::ObjectTypeImpl::ready: static member "
               << it->first << " is not created";

          throw Exception(ostr.str());
        }
        
        if(PyDict_SetItemString(tp_dict, it->first.c_str(), obj) < 0)
        {
          handle_error("El::Python::ObjectTypeImpl::ready");
        }
      }
    }
    
    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    bool
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::check_type(PyObject* ob) throw()
    {
      return ob && ob->ob_type == &TypeClass::instance;
    }

    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    bool
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::is_type(PyObject* ob) throw()
    {
      return ob && ob == reinterpret_cast<PyObject*>(
        static_cast<PyTypeObject*>(&TypeClass::instance));
    }

    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    OBJECT_CLASS*
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::down_cast(PyObject* object,
                                                        bool add_ref,
                                                        bool exception)
      throw(Exception, El::Exception)
    {
      if(object && !check_type(object))
      {
        if(exception)
        {
          std::ostringstream ostr;
          ostr << "object is not of '" << TypeClass::instance.tp_name
               << "' type";
          
          report_error(PyExc_TypeError,
                       ostr.str().c_str(),
                       "El::Python::ObjectTypeImpl::down_cast");
        }
        else
        {
          return 0;
        }
      }

      OBJECT_CLASS* ob = static_cast<OBJECT_CLASS*>(object);
      
      if(add_ref)
      {
        El::Python::add_ref(ob);
      }
        
      return ob;
    }

    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    PyObject*
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::disp_noargs(OBJECT_CLASS* self,
                                                          NoArgsMethod method)
      throw()
    {
      try
      {
        return (self->*method)();
      }
      catch(El::Exception& e)
      {
        El::Python::set_error(e);
      }
      catch(...)
      {
        El::Python::set_runtime_error(
          "El::Python::ObjectTypeImpl::disp_noarg: unknown error");
      }

      return 0;
    }

    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    PyObject*
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::disp_varargs(
      OBJECT_CLASS* self,
      VarArgsMethod method,
      PyObject* args) throw()
    {
      try
      {
        return (self->*method)(args);
      }
      catch(El::Exception& e)
      {
        El::Python::set_error(e);
      }
      catch(...)
      {
        El::Python::set_runtime_error(
          "El::Python::ObjectTypeImpl::disp_varargs: unknown error");
      }

      return 0;      
    }
    
    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    PyObject*
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::disp_kwds(OBJECT_CLASS* self,
                                                        KwdsMethod method,
                                                        PyObject* args,
                                                        PyObject* kwds) throw()
    {
      try
      {
        return (self->*method)(args, kwds);
      }
      catch(El::Exception& e)
      {
        El::Python::set_error(e);
      }
      catch(...)
      {
        El::Python::set_runtime_error(
          "El::Python::ObjectTypeImpl::disp_kwds: unknown error");
      }

      return 0;      
    }
    
    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    PyObject*
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::disp_getter(OBJECT_CLASS* self,
                                                          GetterMethod method)
      throw()
    {
      try
      {
        return (self->*method)();
      }
      catch(El::Exception& e)
      {
        El::Python::set_error(e);
      }
      catch(...)
      {
        El::Python::set_runtime_error(
          "El::Python::ObjectTypeImpl::disp_getter: unknown error");
      }

      return 0;      
    }

    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    int
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::disp_setter(
      OBJECT_CLASS* self,
      SetterMethod method,
      PyObject *value) throw()
    {
      try
      {
        (self->*method)(value);
        return 0;
      }
      catch(El::Exception& e)
      {
        El::Python::set_error(e);
      }
      catch(...)
      {
        El::Python::set_runtime_error(
          "El::Python::ObjectTypeImpl::disp_setter: unknown error");
      }
      
      return -1;
    }

    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    void
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::type_dealloc(PyObject* self)
      throw()
    {
      delete static_cast<OBJECT_CLASS*>(self);
    }

    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    PyObject*
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::type_new(PyTypeObject *type,
                                                       PyObject *args,
                                                       PyObject *kwds)
      throw()
    {
      try
      {
        El::Python::Object_var self = new OBJECT_CLASS(type, args, kwds);
        return self.retn();
        
      }
      catch(El::Exception& e)
      {
        El::Python::set_error(e);
      }
      catch(...)
      {
        El::Python::set_runtime_error("type_new: unknown error");
      }
      
      return 0;
    }

    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    OBJECT_CLASS*
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::create(PyObject* args,
                                                     PyObject* kwds)
      throw(El::Exception)
    {
      return new OBJECT_CLASS(&TypeClass::instance, args, kwds);
    }

    template<typename OBJECT_CLASS, typename TYPE_CLASS>
    PyObject*
    ObjectTypeImpl<OBJECT_CLASS, TYPE_CLASS>::richcompare(PyObject* o1,
                                                          PyObject* o2,
                                                          int op)
      throw()
    {
      try
      {
        if(!check_type(o1) || !check_type(o2))
        {
          return El::Python::add_ref(Py_NotImplemented);
        }

        ObjectClass* ob1 = static_cast<ObjectClass*>(o1);
        ObjectClass* ob2 = static_cast<ObjectClass*>(o2);
        
        ObjectImpl::CMP_RESULT res;
      
        switch(op)
        {
        case Py_LT: res = ob1->lt(ob2); break;
        case Py_LE: res = ob1->le(ob2); break;
        case Py_EQ: res = ob1->eq(ob2); break;
        case Py_NE: res = ob1->ne(ob2); break;
        case Py_GT: res = ob1->gt(ob2); break;
        case Py_GE: res = ob1->ge(ob2); break;
        default:
          {
            std::ostringstream ostr;
            ostr << "El::Python::ObjectTypeImpl::richcompare: unexpected "
              "opcode " << op;

            throw Exception(ostr.str());
          }
        }

        switch(res)
        {
        case ObjectImpl::CR_FALSE:
          {
            return El::Python::add_ref(Py_False);
          }
          
        case ObjectImpl::CR_TRUE: return El::Python::add_ref(Py_True);
        case ObjectImpl::CR_NOT_IMPL:
          return El::Python::add_ref(Py_NotImplemented);
        }
      }
      catch(El::Exception& e)
      {
        set_error(e);
      }

      return 0;
    }
    
    inline
    void
    set_runtime_error(const char* error) throw()
    {
      PyErr_SetString(PyExc_RuntimeError, error);
    }

    inline
    void
    set_index_error(const char* error) throw()
    {
      PyErr_SetString(PyExc_IndexError, error);
    }

    inline
    void
    set_type_error(const char* error) throw()
    {
      PyErr_SetString(PyExc_TypeError, error);
    }

    inline
    void
    set_key_error(const char* error) throw()
    {
      PyErr_SetString(PyExc_KeyError, error);
    }
  }
}

#endif // _ELEMENTS_EL_PYTHON_OBJECT_HPP_
