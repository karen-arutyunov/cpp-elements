/*
 * Elements - useful abstractions library.
 * Copyright (C) 2005-2015 Karen Arutyunov
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

// file      : El/RefCount/Interface.hpp
// author    : Boris Kolpackov <boris@kolpackov.net>
// copyright : Copyright (c) 2002-2003 Boris Kolpackov
// license   : http://kolpackov.net/license.html

#ifndef EL_REFCOUNT_INTERFACE_HPP
#define EL_REFCOUNT_INTERFACE_HPP

#include <El/Exception.hpp>

namespace El
{
  namespace RefCount
  {
    // Interface to a reference-countable object. Note that _remove_ref ()
    // member function has a no-throw semantic. Even though it can lead to
    // a diagnostic loss it was made no-throw because it has a destructor
    // semantic.

    class Interface
    {
    public:
      typedef
      unsigned long
      count_t;

      typedef El::Exception SystemException;
      EL_EXCEPTION(Exception, El::ExceptionBase);

    public:
      virtual void
      add_ref () const throw (Exception, SystemException) = 0;

      virtual void
      remove_ref () const throw () = 0;

      virtual count_t
      refcount_value () const throw (Exception, SystemException) = 0;

    protected:
      Interface () throw ();

      virtual
      ~Interface () throw ();

    protected:
      virtual void
      add_ref_i () const throw (Exception, SystemException) = 0;

      virtual bool
      remove_ref_i () const throw (Exception, SystemException) = 0;

      virtual count_t
      refcount_value_i () const throw (Exception, SystemException) = 0;

    private:
      // Copy semanic is not supported.
      Interface (Interface const&) throw ();
      Interface&
      operator= (Interface const&) throw ();
    };

    template <typename Type>
    Type*
    add_ref (Type* ptr)
      throw (Interface::Exception, Interface::SystemException);

  }
}

#include <El/RefCount/Interface.tpp>
#include <El/RefCount/Interface.ipp>

#endif  // EL_REFCOUNT_COUNTING_INTERFACE_HPP

