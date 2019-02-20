/*

 Copyright (c) 2014, Robin Raymond
 All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are met:

 1. Redistributions of source code must retain the above copyright notice, this
 list of conditions and the following disclaimer.
 2. Redistributions in binary form must reproduce the above copyright notice,
 this list of conditions and the following disclaimer in the documentation
 and/or other materials provided with the distribution.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

 The views and conclusions contained in the software and documentation are those
 of the authors and should not be interpreted as representing official policies,
 either expressed or implied, of the FreeBSD Project.
 
 */

#include <zsLib/internal/zsLib_MessageQueueThread.h>
#include <zsLib/internal/zsLib_MessageQueueThreadBasic.h>
#include <zsLib/internal/zsLib_MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt.h>
#include <zsLib/internal/zsLib_MessageQueueThreadUsingCurrentGUIMessageQueueForWinUWP.h>
#include <zsLib/internal/zsLib_MessageQueueThreadUsingCurrentGUIMessageQueueForWindows.h>
#include <zsLib/internal/zsLib_MessageQueueThreadUsingMainThreadMessageQueueForApple.h>
#include <zsLib/internal/zsLib_MessageQueueThreadUsingBlackberryChannels.h>
#include <zsLib/Log.h>

//namespace zsLib { ZS_DECLARE_SUBSYSTEM(zslib) }

namespace zsLib
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    void setThreadPriority(
                           ZS_MAYBE_USED() Thread::native_handle_type handle,
                           ZS_MAYBE_USED() ThreadPriorities threadPriority
                           ) noexcept
    {
      ZS_MAYBE_USED(handle);
      ZS_MAYBE_USED(threadPriority);
#ifndef _WIN32
      const int policy = SCHED_RR;
      const int minPrio = sched_get_priority_min(policy);
      const int maxPrio = sched_get_priority_max(policy);
      sched_param param;
      switch (threadPriority)
      {
        case ThreadPriority_Idle:
          param.sched_priority = minPrio;
          break;
        case ThreadPriority_Lowest:
          param.sched_priority = minPrio + 1;
          break;
        case ThreadPriority_Low:
          param.sched_priority = minPrio + 2;
          break;
        case ThreadPriority_Normal:
          param.sched_priority = (minPrio + maxPrio) / 2;
          break;
        case ThreadPriority_High:
          param.sched_priority = maxPrio - 3;
          break;
        case ThreadPriority_Highest:
          param.sched_priority = maxPrio - 2;
          break;
        case ThreadPriority_Realtime:
          param.sched_priority = maxPrio - 1;
          break;
      }
      pthread_setschedparam(handle, policy, &param);
#else
		  int priority = THREAD_PRIORITY_NORMAL;
		  switch (threadPriority) {
        case ThreadPriority_Idle:     priority = THREAD_PRIORITY_IDLE; break;
        case ThreadPriority_Lowest:   priority = THREAD_PRIORITY_LOWEST; break;
        case ThreadPriority_Low:      priority = THREAD_PRIORITY_BELOW_NORMAL; break;
        case ThreadPriority_Normal:   priority = THREAD_PRIORITY_NORMAL; break;
        case ThreadPriority_High:     priority = THREAD_PRIORITY_ABOVE_NORMAL; break;
        case ThreadPriority_Highest:  priority = THREAD_PRIORITY_HIGHEST; break;
        case ThreadPriority_Realtime: priority = THREAD_PRIORITY_TIME_CRITICAL; break;
      }
#ifndef WINUWP
		  ZS_MAYBE_USED() auto result = SetThreadPriority(handle, priority);
      ZS_MAYBE_USED(result);
      ZS_ASSERT(0 != result);
#endif //ndef WINUWP

#endif //_WIN32
    }

    //-------------------------------------------------------------------------
    struct StrToPriority
    {
      const char *mStr;
      ThreadPriorities mPriority;
    };

    //-------------------------------------------------------------------------
    static StrToPriority *getPriorities() noexcept
    {
      static StrToPriority gPriorities[] = {
        {
          "idle",
          ThreadPriority_Idle
        },
        {
          "lowest",
          ThreadPriority_Lowest
        },
        {
          "low",
          ThreadPriority_Low
        },
        {
          "normal",
          ThreadPriority_Normal
        },
        {
          "high",
          ThreadPriority_High
        },
        {
          "highest",
          ThreadPriority_Highest
        },
        {
          "real-time",
          ThreadPriority_Realtime
        },
        {
          "real time",
          ThreadPriority_Realtime
        },
        {
          "realtime",
          ThreadPriority_Realtime
        },
        {
          NULL,
          ThreadPriority_Normal
        }
      };
      return gPriorities;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MessageQueueThread
    //

    //-------------------------------------------------------------------------
    MessageQueueThreadPtr MessageQueueThread::createBasic(const char *threadName, ThreadPriorities threadPriority) noexcept
    {
      return internal::MessageQueueThreadBasic::create(threadName, threadPriority);
    }

    //-------------------------------------------------------------------------
    IMessageQueueThreadPtr MessageQueueThread::singletonUsingCurrentGUIThreadsMessageQueue() noexcept
    {
#ifdef _WIN32
#ifdef WINUWP
#ifdef __cplusplus_winrt
      if (internal::MessageQueueThreadUsingCurrentGUIMessageQueueForWinUWP::hasDispatcher()) {
        return internal::MessageQueueThreadUsingCurrentGUIMessageQueueForWinUWP::singleton();
      }
#endif //__cplusplus_winrt
#ifdef CPPWINRT_VERSION
      if (internal::MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt::hasDispatcher()) {
        return internal::MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt::singleton();
      }
#endif //CPPWINRT_VERSION
      return internal::MessageQueueThreadBasic::create("zsLib.backgroundThreadDispatcher");
#else //WINUWP
      return internal::MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::singleton();
#endif //WINUWP
#elif defined(__APPLE__)
      return internal::MessageQueueThreadUsingMainThreadMessageQueueForApple::singleton();
#elif defined(__QNX__)
      return internal::MessageQueueThreadUsingBlackberryChannels::singleton();
#elif defined(ANDROID)
      return internal::MessageQueueThreadBasic::create("build");
#else
#define __STR2__(x) #x
#define __STR1__(x) __STR2__(x)
#define __LOC__ __FILE__ "(" __STR1__(__LINE__) ") : warning: "
#pragma message(__LOC__"Need to implement this method on current target platform...")

      return internal::MessageQueueThreadBasic::create();
#endif //_WIN32
    }


  } // namespace internal

  //---------------------------------------------------------------------------
  const char *toString(ThreadPriorities priority) noexcept
  {
    switch (priority) {
      case ThreadPriority_Lowest:   return "Lowest";
      case ThreadPriority_Low:      return "Low";
      case ThreadPriority_Normal:   return "Normal";
      case ThreadPriority_High:     return "High";
      case ThreadPriority_Highest:  return "Highest";
      case ThreadPriority_Realtime: return "Real-time";
    }
    ZS_ASSERT_FAIL("unknown thread priority");
    return "UNDEFINED";
  }

  using internal::StrToPriority;

  //---------------------------------------------------------------------------
  ThreadPriorities threadPriorityFromString(const char *str) noexcept
  {
    if (!str) return ThreadPriority_Normal;

    String compareTo(str);
    compareTo.trim();

    StrToPriority *priorities = internal::getPriorities();

    for (size_t index = 0; NULL != priorities[index].mStr; ++index)
    {
      String compareStr(priorities[index].mStr);
      if (0 == String(priorities[index].mStr).compareNoCase(compareTo)) {
        return priorities[index].mPriority;
      }
    }

    return ThreadPriority_Normal;
  }

  //---------------------------------------------------------------------------
  void setThreadPriority(
                         Thread &thread,
                         ThreadPriorities threadPriority
                         ) noexcept
  {
    internal::setThreadPriority(thread.native_handle(), threadPriority);
  }

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // MessageQueueThread
  //

  //---------------------------------------------------------------------------
  IMessageQueueThreadPtr IMessageQueueThread::createBasic(
                                                          const char *threadName,
                                                          ThreadPriorities threadPriority
                                                          ) noexcept
  {
    return internal::MessageQueueThread::createBasic(threadName, threadPriority);
  }

  //---------------------------------------------------------------------------
  IMessageQueueThreadPtr IMessageQueueThread::singletonUsingCurrentGUIThreadsMessageQueue() noexcept
  {
    return internal::MessageQueueThread::singletonUsingCurrentGUIThreadsMessageQueue();
  }

}
