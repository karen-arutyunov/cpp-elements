/*
 * product   : Elements - useful abstractions library.
 * copyright : Copyright (c) 2005-2016 Karen Arutyunov
 * licenses  : GNU GPL v2; see accompanying LICENSE file
 *             Commercial; contact karen.arutyunov@gmail.com
 */

/**
 * @file Elements/El/ProcessPool.cpp
 * @author Karen Arutyunov
 * $id:$
 */

#include <sys/types.h>
#include <unistd.h>
#include <poll.h>
#include <fcntl.h>

#include <sstream>
#include <iostream>

#include <ace/OS.h>

#include <El/Exception.hpp>
#include <El/RefCount/All.hpp>
#include <El/BinaryStream.hpp>
#include <El/String/Manip.hpp>
#include <El/IO.hpp>

#include "ProcessPool.hpp"

namespace
{
  const size_t SRV_INFO_SIZE = 100;
}

namespace El
{
  namespace Service
  {
    void
    ProcessPool::run() throw(Exception, El::Exception)
    {
      pid_t pid = 0;
      int input = 0;
      int output = 0;
      bool initial_process_create = true;
      
      while(true)
      {
        try
        {
          Task_var task;

          if(tasks_.dequeue(task))
          {
            bool to_execute = task->execution_required();

            if(!to_execute)
            {
              ReadGuard guard(srv_lock_);
              to_execute = !stop_;
            }

            if(to_execute)
            {
              if(pid == 0)
              {
                std::string error;
                
                pid = create_process(initial_process_create,
                                     input,
                                     output,
                                     error);

                initial_process_create = false;

                if(!pid)
                {
                  task->error(error.c_str());
                }
              }

              bool timeout_occured = false;
              
              if(pid && !execute(task, input, output, timeout_occured))
              {
                std::string error = task->error();
                
                close_process(false,
                              pid,
                              input,
                              output,
                              timeout_occured,
                              error);

                task->timeout_occured(timeout_occured);
                task->error(error.c_str());
              }
            }
            else
            {
              task->error("Service stopped");
            }

            task->signal_completion();
          }
          else
          {
            break;
          }
        }
        catch(const El::Exception& e)
        {
          std::string error = e.what();
          close_process(false, pid, input, output, false, error);
          
          throw;
        }
      }

      std::string error;
      close_process(true, pid, input, output, false, error);
    }

    bool
    ProcessPool::execute(Task* task,
                         const ACE_Time_Value* wait_time,
                         TaskQueue::EnqueueStrategy enqueue_strategy)
      throw(InvalidArg, El::Exception)
    {
      if(task == 0)
      {
        throw InvalidArg("El::Service::ProcessPool::execute: task is null");
      }

      Task_var task_ptr(El::RefCount::add_ref(task));
      
      ReadGuard guard(srv_lock_);

      if(started_ && stop_)
      {
        return false;
      }

      return
        tasks_.enqueue(task_ptr,
                       wait_time,
                       enqueue_strategy == TaskQueue::ES_DEFAULT ?
                       enqueue_strategy_ : enqueue_strategy);
    }

    pid_t
    ProcessPool::create_process(bool initial,
                                int& input,
                                int& output,
                                std::string& error)
      throw(Exception, El::Exception)
    {
      int pipe_out[2];
      int r = pipe(pipe_out);

      if(r)
      {
        int error = ACE_OS::last_error();
                
        std::ostringstream ostr;
        ostr << "El::Service::ProcessPool::create_process: "
          "socketpair failed with code " << error << ". Description:\n"
             << ACE_OS::strerror(error) << std::endl;

        throw Exception(ostr.str());
      }

      int pipe_in[2];
      r = pipe(pipe_in);

      if(r)
      {
        int error = ACE_OS::last_error();
                
        close(pipe_out[0]);
        close(pipe_out[1]);
                
        std::ostringstream ostr;
        ostr << "El::Service::ProcessPool::create_process: "
          "socketpair failed with code " << error << ". Description:\n"
             << ACE_OS::strerror(error) << std::endl;

        throw Exception(ostr.str());
      }

      std::string srv_name = name();
      const char* srv_name_s = srv_name.empty() ? "noname" : srv_name.c_str();

      std::string in = El::String::Manip::string(pipe_out[0]);
      const char* in_s = in.c_str();
      
      std::string out = El::String::Manip::string(pipe_in[1]);
      const char* out_s = out.c_str();
        
      pid_t pid = fork();

      if(pid == 0)
      {
//        dup2(pipe_out[0], STDIN_FILENO);
//        dup2(pipe_in[1], STDOUT_FILENO);

        int r = execlp("ElPoolProcess",
                       "ElPoolProcess",
                       "exec",
                       srv_name_s,
                       in_s,
                       out_s,
                       NULL);

        if(r)
        {
          int error = ACE_OS::last_error();

          std::cerr << "El::Service::ProcessPool::create_process: execlp "
            "failed with code " << error << ". Description:\n"
                    << ACE_OS::strerror(error) << std::endl;
                  
          exit(-1);
        }

        std::cerr << "El::Service::ProcessPool::create_process: "
          "shouldn't be here\n";
     
        exit(-1);
      }
              
      close(pipe_out[0]);
      output = pipe_out[1];

      close(pipe_in[1]);
      input = pipe_in[0];

//      std::cerr << "* " << output << " / " << input << std::endl;

      std::ostringstream ostr;
      El::BinaryOutStream bstr(ostr);

      bstr << "INIT" << factory_lib_ << factory_func_
           << factory_args_ << extra_libs_;

      std::string data;
      bool timeout_occured = false;
      
      if(!send_command(ostr.str(),
                       output,
                       input,
                       data,
                       timeout_occured))
      {
        close_process(false, pid, input, output, timeout_occured, data);
        error = data;
        
        return 0;
      }
      
      {
        std::istringstream istr(data);
        El::BinaryInStream bstr(istr);

        if(check_error_result(bstr, error))
        {
          close_process(false, pid, input, output, false, error);
          return 0;
        }
      }

      if(!initial)
      {
        std::ostringstream ostr;
        ostr << "El::Service::ProcessPool::create_process: pid " << pid;
          
        El::Service::Error error(ostr.str(), this, El::Service::Error::NOTICE);
        callback_->notify(&error);
      }

      return pid;
    }

    bool
    ProcessPool::write(int fd,
                       const void* buff,
                       size_t count,
                       std::string& error,
                       bool& timeout_occured) const
      throw(El::Exception)
    {
      timeout_occured = false;
      error.clear();
      
      const char* ptr = (const char*)buff;
      const char* end = ptr + count;
      bool success = true;

      pollfd pfd;
      
      pfd.fd = fd;
      pfd.events = POLLOUT;
        
      while(success && ptr < end)
      {
        pfd.revents = 0;
        int r = poll(&pfd, 1, timeout_);

        if(r < 0)
        {
          if(errno == EINTR)
          {
            continue;
          }
          
          success = false;
          break;
        }

        if(r == 0)
        {
          error = "Write timeout";
          timeout_occured = true;
          return false;
        }
        
        ssize_t size =
          El::write(fd,
                    (void*)ptr,
                    std::min((size_t)PIPE_BUF, (size_t)(end - ptr)));
        
        if(size <= 0)
        {
          success = false;
        }
        else
        {
          ptr += size;
        }
      }

      if(!success)
      {
        int e = ACE_OS::last_error();    
          
        std::ostringstream ostr;
        ostr << "write failed with code " << e;

        if(e)
        {
          ostr << "; description: " << ACE_OS::strerror(e);
        }

        error = ostr.str();
        return false;
      }

      return true;
    }
    
    bool
    ProcessPool::read(int fd,
                      void* buff,
                      size_t count,
                      std::string& error,
                      bool& timeout_occured) const
      throw(El::Exception)
    {
      timeout_occured = false;
      error.clear();

      char* ptr = (char*)buff;
      const char* end = ptr + count;
      bool success = true;

      pollfd pfd;
      
      pfd.fd = fd;
        
      while(success && ptr < end)
      {
        pfd.events = POLLIN | POLLPRI | POLLRDBAND | POLLRDNORM;
        pfd.revents = 0;
        
        int r = poll(&pfd, 1, timeout_);

        if(r < 0)
        {
          if(errno == EINTR)
          {
            continue;
          }

          success = false;
          break;
        }

        if(r == 0)
        {
          error = "Read timeout";
          timeout_occured = true;
          return false;
        }
        
        ssize_t size =
          El::read(fd, ptr, std::min((size_t)PIPE_BUF, (size_t)(end - ptr)));
        
        if(size <= 0)
        {
          success = false;
        }
        else
        {
          ptr += size;
        }
      }

      if(!success)
      {
        int e = ACE_OS::last_error();    
          
        std::ostringstream ostr;
        ostr << "read failed with code " << e;

        if(e)
        {
          ostr << "; description: " << ACE_OS::strerror(e);
        }

        error = ostr.str();
        return false;
      }

      return true;      
    }
    
    bool
    ProcessPool::send_command(const std::string& data,
                              int output,
                              int input,
                              std::string& result,
                              bool& timeout_occured) const
      throw(El::Exception)
    {
      timeout_occured = false;

      uint32_t signature = rand();
      size_t data_len = data.length();
      
      bool success =
        write(output, &signature, sizeof(signature), result, timeout_occured)&&
        write(output, &data_len, sizeof(data_len), result, timeout_occured) &&
        write(output, data.c_str(), data_len, result, timeout_occured);
              
      if(!success)
      {
        return false;
      }

      uint32_t res_signature = 0;

      if(!read(input,
               &res_signature,
               sizeof(res_signature),
               result,
               timeout_occured))
      {
        return false;
      }
      
      if(signature != res_signature)
      {
        result = "Unexpected response signature";
        return false;
      }

      std::ostringstream ostr;

      data_len = 0;
      
      success = read(input,
                     &data_len,
                     sizeof(data_len),
                     result,
                     timeout_occured);

      if(success)
      {
        size_t max_result_len = SIZE_MAX - max_result_len_ < SRV_INFO_SIZE ?
          SIZE_MAX : (max_result_len_ + SRV_INFO_SIZE);
          
        if(data_len > std::max(max_result_len, (size_t)10240 + SRV_INFO_SIZE))
        {
          // 10240 - max exception description len, 100 - size of service info

          std::ostringstream ostr;
          ostr << "Max allowed result length (" << max_result_len_
               << ") exceeded: " << data_len;
            
          result = ostr.str();
          return false;
        }
        
        char buff[1024 * 10];
        size_t size = 0;
        
        while(data_len &&
              (success = read(input,
                              buff,
                              size = std::min(data_len, sizeof(buff)),
                              result,
                              timeout_occured)))
        {
          ostr.write(buff, size);
          data_len -= size;
        }
      }
      
      if(success)
      {
        result = ostr.str();
      }

      return success;
    }

    bool
    ProcessPool::check_error_result(El::BinaryInStream& bstr,
                                    std::string& error)
      throw(Exception, El::Exception)
    {
      std::string res;
      bstr >> res;

      if(res == "E")
      {
        bstr >> error;
        return true;
      }
      else if(res == "S")
      {
        return false;
      }

      std::ostringstream ostr;
      ostr << "El::Service::ProcessPool::error_result: invalid result flag '"
           << res << "'";

      throw Exception(ostr.str());
    }
    
    bool
    ProcessPool::execute(Task* task,
                         int input,
                         int output,
                         bool& timeout_occured) const
      throw(El::Exception)
    {
      std::ostringstream ostr;

      try
      {
        El::BinaryOutStream bstr(ostr);
        bstr << "TASK" << task->type_id();
        task->write_arg(bstr);
      }
      catch(const El::Exception& e)
      {
        std::ostringstream ostr;          
        ostr << "Failed to write task arguments. Details: " << e;

        task->error(ostr.str().c_str());
        return false;
      }

      std::string data;
      
      bool res = send_command(ostr.str(),
                              output,
                              input,
                              data,
                              timeout_occured);
      
      if(res)
      {
        std::istringstream istr(data);
        El::BinaryInStream bstr(istr);
        
        try
        {
          std::string error;
        
          if(check_error_result(bstr, error))
          {
            task->error(error.c_str());
          }
          else
          {
            task->read_res(bstr);
          }
        }
        catch(const El::Exception& e)
        {
          res = false;
          
          std::ostringstream ostr;
          
          ostr << "Broken protocol. Bytes read " << bstr.read_bytes()
               << " from " << data.length() << ". Details: " << e
               << "\nBase64 dump:";

          El::String::Manip::base64_encode((const unsigned char*)data.c_str(),
                                           data.length(),
                                           ostr);
          
          task->error(ostr.str().c_str());
        }
      }
      else
      {
        task->error(data.c_str());
      }

      return res;
    }
    
    bool
    ProcessPool::close_process(bool regular,
                               pid_t& pid,
                               int& input,
                               int& output,
                               bool timeout_occured,
                               std::string& error)
      throw(El::Exception)
    {
      if(input)
      {
        close(input);
        input = 0;
      }
      
      if(output)
      {
        close(output);
        output = 0;
      }

      pid_t closed_pid = pid;
      std::string close_error;
      
      if(pid)
      {
        if(timeout_occured)
        {
          kill(pid, SIGKILL);
        }
        
        int status = 0;
        pid_t ecode = waitpid(pid, &status, 0);
        bool success = ecode == pid;
        
        pid = 0;
          
        if(success)
        {
          if(!WIFEXITED(status))
          {
            close_error = "Child abnormal termination";
          }
        }
        else
        {
          int error = ACE_OS::last_error();    
          
          std::ostringstream ostr;
          ostr << "waitpid failed with code " << error << ". Description:\n"
               << ACE_OS::strerror(error);
          
          close_error = ostr.str();
        }

        if(close_error.empty())
        {
          status = WEXITSTATUS(status);
        
          if(status)
          {
            std::ostringstream ostr;
            ostr << "Child exited with code " << status;
          
            close_error = ostr.str();
          }
        }

        if(!close_error.empty())
        {
          error += "; " + close_error;
        }
      }

      if(!regular)
      {
        std::ostringstream ostr;
        ostr << "El::Service::ProcessPool::close_process: pid " << closed_pid
             << ", reason " << (error.empty() ? "<none>" : error);
          
        El::Service::Error error(ostr.str(), this, El::Service::Error::NOTICE);
        callback_->notify(&error);
      }
      
      return close_error.empty();
    }
  }
  
}
