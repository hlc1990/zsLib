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

#include <zsLib/internal/zsLib_MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt.h>
#include <zsLib/internal/zsLib_MessageQueueDispatcherForCppWinrt.h>
#include <zsLib/internal/zsLib_MessageQueueDispatcher.h>
#include <zsLib/internal/zsLib_MessageQueueThreadBasic.h>
#include <zsLib/internal/zsLib_MessageQueue.h>
#include <zsLib/Log.h>
#include <zsLib/helpers.h>
#include <zsLib/Stringize.h>
#include <zsLib/Singleton.h>

#ifdef CPPWINRT_VERSION

using namespace winrt::Windows::UI::Core;
typedef winrt::Windows::System::DispatcherQueue DispatcherQueue;

namespace zsLib { ZS_DECLARE_SUBSYSTEM(zslib) }

namespace zsLib
{
  namespace internal
  {

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

    //-------------------------------------------------------------------------
    static IMessageQueueThreadPtr CoreDispatcherSingleton() noexcept
    {
      if (MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt::hasCoreDispatcher()) {
        CoreDispatcher dispatcher = MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt::setupCoreDispatcher();

        if (dispatcher) {
          static SingletonLazySharedPtr<MessageQueueDispatcherForCppWinrt> singleton(MessageQueueDispatcherForCppWinrt::create(dispatcher));
          return singleton.singleton();
        }
      }
      return IMessageQueueThreadPtr();
    }


    //-------------------------------------------------------------------------
    static IMessageQueueThreadPtr DispatcherQueueSingleton() noexcept
    {
      if (MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt::hasDispatcherQueue()) {
        DispatcherQueue dispatcher = MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt::setupDispatcherQueue();

        if (dispatcher) {
          static SingletonLazySharedPtr<MessageQueueDispatcherForCppWinrt> singleton(MessageQueueDispatcherForCppWinrt::create(dispatcher));
          return singleton.singleton();
        }
      }
      return IMessageQueueThreadPtr();
    }

    //-------------------------------------------------------------------------
    IMessageQueueThreadPtr MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt::singleton() noexcept
    {
      {
        auto result = CoreDispatcherSingleton();
        if (result)
          return result;
      }
      {
        auto result = DispatcherQueueSingleton();
        if (result)
          return result;
      }

      if (!hasDispatcherQueue()) {
        auto dispatcher = DispatcherQueue::GetForCurrentThread();
        if (dispatcher) {
          setupDispatcherQueue(dispatcher);
        }
      }
      {
        auto result = DispatcherQueueSingleton();
        if (result)
          return result;
      }

      if (!hasCoreDispatcher()) {
        auto window = winrt::Windows::UI::Core::CoreWindow::GetForCurrentThread();
        auto dispatcher = window.Dispatcher();
        if (dispatcher) {
          setupCoreDispatcher(dispatcher);
        }
      }
      {
        auto result = CoreDispatcherSingleton();
        if (result)
          return result;
      }

      static SingletonLazySharedPtr<MessageQueueThreadBasic> singleton(MessageQueueThreadBasic::create("zsLib.cppwinrt.backgroundDispatcher"));
      return singleton.singleton();
    }

    //-------------------------------------------------------------------------
    CoreDispatcher MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt::setupCoreDispatcher(CoreDispatcher dispatcher) noexcept
    {
      static CoreDispatcher gDispatcher = dispatcher;
      if (dispatcher)
        MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt::hasCoreDispatcher(true);
      return gDispatcher;
    }

    //-------------------------------------------------------------------------
    DispatcherQueue MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt::setupDispatcherQueue(DispatcherQueue dispatcher) noexcept
    {
      static DispatcherQueue gDispatcher = dispatcher;
      if (dispatcher)
        MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt::hasDispatcherQueue(true);
      return gDispatcher;
    }

    //-------------------------------------------------------------------------
    bool MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt::hasCoreDispatcher(bool ready) noexcept
    {
      static std::atomic<bool> isSetup {false};
      if (ready) isSetup = true;
      return isSetup;
    }

    //-------------------------------------------------------------------------
    bool MessageQueueThreadUsingCurrentGUIMessageQueueForCppWinrt::hasDispatcherQueue(bool ready) noexcept
    {
      static std::atomic<bool> isSetup {false};
      if (ready) isSetup = true;
      return isSetup;
    }
  }
}

#endif //CPPWINRT_VERSION
