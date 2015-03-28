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

/**
 * @file   Elements/tests/ProcessPool/Task.hpp
 * @author Karen Arutyunov
 * $Id:$
 */

#ifndef _ELEMENTS_TESTS_PROCESSPOOL_TASK_HPP_
#define _ELEMENTS_TESTS_PROCESSPOOL_TASK_HPP_

#include <stdint.h>

#include <string>
#include <list>

#include <ace/OS.h>
#include <ace/Synch.h>
#include <ace/Guard_T.h>

#include <El/Exception.hpp>

#include <El/Service/Service.hpp>
#include <El/Service/ProcessPool.hpp>

struct DoublingTask : public virtual El::Service::ProcessPool::TaskBase
{
  uint32_t param;
  uint32_t result;
    
  DoublingTask(unsigned long prm = 0) throw(El::Exception);

  virtual const char* type_id() const throw(El::Exception) { return "Dbl"; }
    
  virtual void execute() throw(El::Exception);

  virtual void write_arg(El::BinaryOutStream& bstr) const
    throw(El::Exception);
        
  virtual void read_arg(El::BinaryInStream& bstr) throw(El::Exception);
  virtual void write_res(El::BinaryOutStream& bstr) throw(El::Exception);
        
  virtual void read_res(El::BinaryInStream& bstr) throw(El::Exception);
};

typedef El::RefCount::SmartPtr<DoublingTask> DoublingTask_var;

///////////////////////////////////////////////////////////////////////////////
// Inlines
///////////////////////////////////////////////////////////////////////////////

//
// DoublingTask class
//
inline
DoublingTask::DoublingTask(unsigned long prm) throw(El::Exception)
    : El::Service::ProcessPool::TaskBase(true),
      param(prm),
      result(UINT32_MAX)
{
}

inline
void
DoublingTask::execute() throw(El::Exception)
{
  result = param * 2;
}

inline
void
DoublingTask::write_arg(El::BinaryOutStream& bstr) const throw(El::Exception)
{
  bstr << param;
}

inline
void
DoublingTask::read_arg(El::BinaryInStream& bstr) throw(El::Exception)
{
  bstr >> param;
}

inline
void
DoublingTask::write_res(El::BinaryOutStream& bstr) throw(El::Exception)
{
  bstr << result;
}
        
inline
void
DoublingTask::read_res(El::BinaryInStream& bstr) throw(El::Exception)
{
  bstr >> result;
}

#endif // _ELEMENTS_TESTS_PROCESSPOOL_TASK_HPP_
