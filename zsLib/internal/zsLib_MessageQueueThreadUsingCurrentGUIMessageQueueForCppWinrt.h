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

#pragma once

#ifdef WINUWP

#ifdef __has_include
#if __has_include(<winrt/windows.ui.core.h>)
#include <winrt/windows.ui.core.h>
#endif //__has_include(<winrt/windows.ui.core.h>)
#endif //__has_include

#ifdef CPPWINRT_VERSION

#include <Windows.h>

#include <zsLib/internal/zsLib_MessageQueueThread.h>

#include <zsLib/Exception.h>

namespace zsLib
{
  namespace internal
  {
    ZS_DECLARE_CLASS_PTR(MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt);

    class MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt : public MessageQueueThread,
                                                                    public IMessageQueueNotify
    {
    public:
      typedef winrt::Windows::UI::Core::CoreDispatcher CoreDispatcher;

      struct Exceptions
      {
        ZS_DECLARE_CUSTOM_EXCEPTION(MessageQueueAlreadyDeleted)
      };

    protected:
      MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt() noexcept;
      static MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrtPtr create(CoreDispatcher dispatcher) noexcept;
      static void dispatch(MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrtPtr queue) noexcept;

    public:
      ~MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt() noexcept;

      static MessageQueueThreadPtr singleton() noexcept;
      static CoreDispatcher setupDispatcher(CoreDispatcher dispatcher = nullptr) noexcept;
      static bool hasDispatcher(bool ready = false) noexcept;

      // IMessageQueue
      virtual void post(IMessageQueueMessageUniPtr message) noexcept(false);

      virtual size_type getTotalUnprocessedMessages() const noexcept;

      virtual bool isRunningOnUIThread() const noexcept;

      // IMessageQueueNotify
      virtual void notifyMessagePosted() noexcept;

      // IMessageQueueThread
      virtual void waitForShutdown() noexcept;

      virtual void setThreadPriority(ThreadPriorities threadPriority) noexcept;

    public:
      virtual void process() noexcept;
      virtual void processMessagesFromThread() noexcept;

    protected:
      mutable Lock mLock;
      MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrtWeakPtr mThisWeak;

      MessageQueuePtr mQueue;
      CoreDispatcher mDispatcher {nullptr};

      std::atomic_bool mIsShutdown {};
    };
  }
}

#endif //CPPWINRT_VERSION

#endif //WINUWP
