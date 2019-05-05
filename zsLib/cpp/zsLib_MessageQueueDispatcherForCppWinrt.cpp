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

#include <zsLib/types.h>
#include <zsLib/internal/platform.h>

#include <zsLib/internal/zsLib_MessageQueueDispatcherForCppWinrt.h>

#ifdef CPPWINRT_VERSION

#include <zsLib/internal/zsLib_MessageQueueThreadBasic.h>
#include <zsLib/internal/zsLib_MessageQueue.h>
#include <zsLib/Log.h>
#include <zsLib/helpers.h>
#include <zsLib/Stringize.h>
#include <zsLib/Singleton.h>

#include <Windows.h>
#include <tchar.h>

#include <sdkddkver.h>

namespace zsLib { ZS_DECLARE_SUBSYSTEM(zslib) }

namespace zsLib
{
  namespace internal
  {
    //-------------------------------------------------------------------------
    static winrt::Windows::UI::Core::CoreDispatcherPriority convertCoreDispatcher(ThreadPriorities threadPriority)
    {
      switch (threadPriority)
      {
          case ThreadPriority_Idle:     return winrt::Windows::UI::Core::CoreDispatcherPriority::Idle;
          case ThreadPriority_Lowest:   
          case ThreadPriority_Low:      return winrt::Windows::UI::Core::CoreDispatcherPriority::Low;
          case ThreadPriority_Normal:   return winrt::Windows::UI::Core::CoreDispatcherPriority::Normal;
          case ThreadPriority_High:
          case ThreadPriority_Highest:
          case ThreadPriority_Realtime: return winrt::Windows::UI::Core::CoreDispatcherPriority::High;
      }
      return winrt::Windows::UI::Core::CoreDispatcherPriority::Normal;
    }

    //-------------------------------------------------------------------------
    static winrt::Windows::System::DispatcherQueuePriority convertDispatcherQueue(ThreadPriorities threadPriority)
    {
      switch (threadPriority)
      {
          case ThreadPriority_Idle:     
          case ThreadPriority_Lowest:   
          case ThreadPriority_Low:      return winrt::Windows::System::DispatcherQueuePriority::Low;
          case ThreadPriority_Normal:   return winrt::Windows::System::DispatcherQueuePriority::Normal;
          case ThreadPriority_High:
          case ThreadPriority_Highest:
          case ThreadPriority_Realtime: return winrt::Windows::System::DispatcherQueuePriority::High;
      }
      return winrt::Windows::System::DispatcherQueuePriority::Normal;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    void MessageQueueDispatcherForCppWinrt::dispatch(MessageQueueDispatcherForCppWinrtPtr queue) noexcept
    {
      queue->process();
    }

    //-------------------------------------------------------------------------
    MessageQueueDispatcherForCppWinrt::MessageQueueDispatcherForCppWinrt() noexcept
    {
    }

    //-------------------------------------------------------------------------
    MessageQueueDispatcherForCppWinrt::~MessageQueueDispatcherForCppWinrt() noexcept
    {
      thisWeak_.reset();
      coreDispatcher_ = nullptr;
      dispatcherQueue_ = nullptr;
    }

    //-------------------------------------------------------------------------
    MessageQueueDispatcherForCppWinrtPtr MessageQueueDispatcherForCppWinrt::create(
      CoreDispatcher dispatcher,
      ThreadPriorities threadPriority
      ) noexcept
    {
      MessageQueueDispatcherForCppWinrtPtr thread(new MessageQueueDispatcherForCppWinrt);
      thread->thisWeak_ = thread;
      thread->queue_ = MessageQueue::create(thread);
      thread->coreDispatcher_ = dispatcher;
      thread->coreDispatcherPriority_ = convertCoreDispatcher(threadPriority);
      ZS_ASSERT(thread->coreDispatcher_);
      return thread;
    }

    //-------------------------------------------------------------------------
    MessageQueueDispatcherForCppWinrtPtr MessageQueueDispatcherForCppWinrt::create(
      DispatcherQueue dispatcher,
      ThreadPriorities threadPriority
      ) noexcept
    {
      MessageQueueDispatcherForCppWinrtPtr thread(new MessageQueueDispatcherForCppWinrt);
      thread->thisWeak_ = thread;
      thread->queue_ = MessageQueue::create(thread);
      thread->dispatcherQueue_ = dispatcher;
      thread->dispatcherQueuePriority_ = convertDispatcherQueue(threadPriority);
      ZS_ASSERT(thread->dispatcherQueue_);
      return thread;
    }

    //-------------------------------------------------------------------------
    void MessageQueueDispatcherForCppWinrt::process() noexcept
    {
      queue_->processOnlyOneMessage(); // process only one message at a time since this must be syncrhonized through the GUI message queue
    }

    //-------------------------------------------------------------------------
    void MessageQueueDispatcherForCppWinrt::processMessagesFromThread() noexcept
    {
      queue_->process();
    }

    //-------------------------------------------------------------------------
    void MessageQueueDispatcherForCppWinrt::post(IMessageQueueMessageUniPtr message) noexcept(false)
    {
      if (isShutdown_) {
        ZS_THROW_CUSTOM(IMessageQueue::Exceptions::MessageQueueGone, "message posted to message queue after message queue was deleted.")
      }
      queue_->post(std::move(message));
    }

    //-------------------------------------------------------------------------
    IMessageQueue::size_type MessageQueueDispatcherForCppWinrt::getTotalUnprocessedMessages() const noexcept
    {
      return queue_->getTotalUnprocessedMessages();
    }

    //-------------------------------------------------------------------------
    bool MessageQueueDispatcherForCppWinrt::isCurrentThread() const noexcept
    {
      ZS_ASSERT((nullptr != coreDispatcher_) ||
                (nullptr != dispatcherQueue_));

      if (coreDispatcher_) {
        return coreDispatcher_.HasThreadAccess();
      }
#ifdef NTDDI_WIN10_RS5
#if (NTDDI_VERSION > NTDDI_WIN10_RS5)
      return dispatcherQueue_.HasThreadAccess();
#else
      return false;
#endif (NTDDI_VERSION > NTDDI_WIN10_RS5)
#else
      return false;
#endif //ndef NTDDI_WIN10_RS5
    }

    //-------------------------------------------------------------------------
    void MessageQueueDispatcherForCppWinrt::notifyMessagePosted() noexcept
    {
      ZS_ASSERT((nullptr != coreDispatcher_) ||
                (nullptr != dispatcherQueue_));

      auto queue = thisWeak_.lock();

      winrt::Windows::UI::Core::CoreDispatcherPriority coreDispatcherPriority {};
      winrt::Windows::System::DispatcherQueuePriority  dispatcherQueuePriority {};

      {
        AutoLock lock(lock_);
        coreDispatcherPriority = coreDispatcherPriority_;
        dispatcherQueuePriority = dispatcherQueuePriority_;
      }

      auto callback = [queue] () {dispatch(queue);};

      if (coreDispatcher_) {
        coreDispatcher_.RunAsync(
          coreDispatcherPriority,
          winrt::Windows::UI::Core::DispatchedHandler(callback)
        );
      } else {
        dispatcherQueue_.TryEnqueue(
          dispatcherQueuePriority,
          winrt::Windows::System::DispatcherQueueHandler(callback)
        );
      }
    }

    //-------------------------------------------------------------------------
    void MessageQueueDispatcherForCppWinrt::waitForShutdown() noexcept
    {
      isShutdown_ = true;
    }

    //-------------------------------------------------------------------------
    void MessageQueueDispatcherForCppWinrt::setThreadPriority(ThreadPriorities threadPriority) noexcept
    {
      AutoLock lock(lock_);
      coreDispatcherPriority_ = convertCoreDispatcher(threadPriority);
      dispatcherQueuePriority_ = convertDispatcherQueue(threadPriority);
    }
  }
}

#endif //CPPWINRT_VERSION
