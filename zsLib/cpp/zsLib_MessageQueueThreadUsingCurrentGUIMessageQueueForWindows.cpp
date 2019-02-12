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

#if defined(_WIN32) && !defined(WINUWP) && !defined(WIN32_RX64)

#include <zsLib/internal/zsLib_MessageQueueThreadUsingCurrentGUIMessageQueueForWindows.h>
#include <zsLib/internal/zsLib_MessageQueue.h>
#include <zsLib/Log.h>
#include <zsLib/helpers.h>
#include <zsLib/Stringize.h>
#include <zsLib/Singleton.h>

#include <Windows.h>
#include <tchar.h>

#pragma warning(disable: 4297)

namespace zsLib { ZS_DECLARE_SUBSYSTEM(zslib) }

namespace zsLib
{
  namespace internal
  {

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

    static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept;

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    class MessageQueueThreadUsingCurrentGUIMessageQueueForWindowsGlobal
    {
    public:
      MessageQueueThreadUsingCurrentGUIMessageQueueForWindowsGlobal() noexcept :
        mRegisteredWindowClass(0),
        mHiddenWindowClassName(_T("zsLibHiddenWindowe059928c0dab4631bdaeab09d5b25847")),
        mCustomMessageName(_T("zsLibCustomMessage3bbf9fd89d067b42860cc9074d64539f")),
        mModule(::GetModuleHandle(NULL))
      {
        WNDCLASS wndClass;
        memset(&wndClass, 0, sizeof(wndClass));

        wndClass.style = CS_HREDRAW | CS_VREDRAW;
        wndClass.lpfnWndProc = windowProc;
        wndClass.cbClsExtra = 0;
        wndClass.cbWndExtra = 0;
        wndClass.hInstance = mModule;
        wndClass.hIcon = NULL;
        wndClass.hCursor = LoadCursor(NULL, IDC_ARROW);
        wndClass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
        wndClass.lpszMenuName = NULL;
        wndClass.lpszClassName = mHiddenWindowClassName;

        mRegisteredWindowClass = ::RegisterClass(&wndClass);
        ZS_ASSERT(0 != mRegisteredWindowClass);

        mCustomMessage = ::RegisterWindowMessage(mCustomMessageName);
        ZS_ASSERT(0 != mCustomMessage);
      }

      ~MessageQueueThreadUsingCurrentGUIMessageQueueForWindowsGlobal() noexcept
      {
        if (0 != mRegisteredWindowClass)
        {
          auto result = ::UnregisterClass(mHiddenWindowClassName, mModule);
          ZS_MAYBE_USED(result);
          ZS_ASSERT(0 != result);
          mRegisteredWindowClass = 0;
        }
      }

    public:
      HMODULE mModule;
      ATOM mRegisteredWindowClass;
      const TCHAR *mHiddenWindowClassName;
      const TCHAR *mCustomMessageName;
      UINT mCustomMessage;
    };

    //-------------------------------------------------------------------------
    static MessageQueueThreadUsingCurrentGUIMessageQueueForWindowsGlobal &getGlobal() noexcept
    {
      static MessageQueueThreadUsingCurrentGUIMessageQueueForWindowsGlobal global;
      return global;
    }

    //-------------------------------------------------------------------------
    MessageQueueThreadUsingCurrentGUIMessageQueueForWindowsPtr MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::singleton() noexcept
    {
      static SingletonLazySharedPtr<MessageQueueThreadUsingCurrentGUIMessageQueueForWindows> singleton(MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::create());
      return singleton.singleton();
    }

    //-------------------------------------------------------------------------
    MessageQueueThreadUsingCurrentGUIMessageQueueForWindowsPtr MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::create() noexcept
    {
      MessageQueueThreadUsingCurrentGUIMessageQueueForWindowsPtr thread(new MessageQueueThreadUsingCurrentGUIMessageQueueForWindows);
      thread->mQueue = MessageQueue::create(thread);
      thread->setup();
      return thread;
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::setup() noexcept
    {
      // make sure the window message queue was created by peeking a message
      MSG msg;
      memset(&msg, 0, sizeof(msg));
      ::PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE);

      mHWND = ::CreateWindow(
        getGlobal().mHiddenWindowClassName,
        _T("zsLibHiddenWindow9127b0cb49457fdb969054c57cff6ed5efe771c0"),
        0,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        CW_USEDEFAULT,
        NULL,
        NULL,
        getGlobal().mModule,
        NULL
      );
      ZS_ASSERT(NULL != mHWND);
    }

    //-------------------------------------------------------------------------
    MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::MessageQueueThreadUsingCurrentGUIMessageQueueForWindows() noexcept :
      mHWND(NULL)
    {
    }

    //-------------------------------------------------------------------------
    MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::~MessageQueueThreadUsingCurrentGUIMessageQueueForWindows() noexcept
    {
      if (NULL != mHWND)
      {
        auto result = ::DestroyWindow(mHWND);
        ZS_MAYBE_USED(result);
        ZS_ASSERT(0 != result);
        mHWND = NULL;
      }
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::process() noexcept
    {
      mQueue->processOnlyOneMessage(); // process only one message at a time since this must be syncrhonized through the GUI message queue
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::processMessagesFromThread() noexcept
    {
      mQueue->process();
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::post(IMessageQueueMessageUniPtr message) noexcept(false)
    {
      if (mIsShutdown) {
        ZS_ASSERT_FAIL("message posted to message queue after message queue was deleted.");
      }
      mQueue->post(std::move(message));
    }

    //-------------------------------------------------------------------------
    IMessageQueue::size_type MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::getTotalUnprocessedMessages() const noexcept
    {
      return mQueue->getTotalUnprocessedMessages();
    }


    //-------------------------------------------------------------------------
    bool MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::isCurrentThread() const noexcept
    {
      return FALSE != ::IsGUIThread(FALSE);
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::notifyMessagePosted() noexcept
    {
      AutoLock lock(mLock);
      if (NULL == mHWND) {
        ZS_ASSERT_FAIL("message posted to message queue after message queue was deleted.");
      }

      auto result = ::PostMessage(mHWND, getGlobal().mCustomMessage, (WPARAM)0, (LPARAM)0);
      ZS_MAYBE_USED(result);
      ZS_ASSERT(0 != result);
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::waitForShutdown() noexcept
    {
      AutoLock lock(mLock);

      if (NULL != mHWND)
      {
        auto result = ::DestroyWindow(mHWND);
        ZS_MAYBE_USED(result);
        ZS_ASSERT(0 != result);
        mHWND = NULL;
      }

      mIsShutdown = true;
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::setThreadPriority(ZS_MAYBE_USED() ThreadPriorities threadPriority) noexcept
    {
      ZS_MAYBE_USED(threadPriority);
      // no-op
    }

    //-------------------------------------------------------------------------
    static LRESULT CALLBACK windowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) noexcept
    {
      if (getGlobal().mCustomMessage != uMsg)
        return ::DefWindowProc(hWnd, uMsg, wParam, lParam);

      MessageQueueThreadUsingCurrentGUIMessageQueueForWindowsPtr singleton = MessageQueueThreadUsingCurrentGUIMessageQueueForWindows::singleton();
      singleton->process();
      return (LRESULT)0;
    }
  }
}

#endif //defined(_WIN32) && !defined(WINUWP)
