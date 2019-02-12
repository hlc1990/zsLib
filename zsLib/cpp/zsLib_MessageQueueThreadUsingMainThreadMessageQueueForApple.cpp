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

#ifdef __APPLE__

#include <zsLib/internal/zsLib_MessageQueueThreadUsingMainThreadMessageQueueForApple.h>
#include <zsLib/internal/zsLib_MessageQueue.h>

#include <zsLib/Log.h>
#include <zsLib/helpers.h>
#include <zsLib/Stringize.h>
#include <zsLib/Singleton.h>


namespace zsLib { ZS_DECLARE_SUBSYSTEM(zslib) }

namespace zsLib
{
  namespace internal
  {
    // These are the CFRunLoopSourceRef callback functions.
    void RunLoopSourcePerformRoutine (void *info);
    void MoreMessagesLoopSourcePerformRoutine (void *info);

    //-------------------------------------------------------------------------
    MessageQueueThreadUsingMainThreadMessageQueueForApplePtr MessageQueueThreadUsingMainThreadMessageQueueForApple::singleton() noexcept
    {
      static SingletonLazySharedPtr<MessageQueueThreadUsingMainThreadMessageQueueForApple> singleton(MessageQueueThreadUsingMainThreadMessageQueueForApple::create());
      MessageQueueThreadUsingMainThreadMessageQueueForApplePtr result = singleton.singleton();
      if (!result) {
        ZS_LOG_WARNING(Detail, slog("singleton gone"))
      }
      return result;
    }

    //-------------------------------------------------------------------------
    MessageQueueThreadUsingMainThreadMessageQueueForApplePtr MessageQueueThreadUsingMainThreadMessageQueueForApple::create() noexcept
    {
      MessageQueueThreadUsingMainThreadMessageQueueForApplePtr thread(new MessageQueueThreadUsingMainThreadMessageQueueForApple);
      thread->mQueue = MessageQueue::create(thread);
      thread->init();
      return thread;
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingMainThreadMessageQueueForApple::init() noexcept
    {
      //CFRunLoopSourceContext context = {0, this, NULL, NULL, NULL, NULL, NULL, &RunLoopSourceScheduleRoutine,RunLoopSourceCancelRoutine, RunLoopSourcePerformRoutine};
      mRunLoop = CFRunLoopGetMain();

      CFRunLoopSourceContext context = {0, this, NULL, NULL, NULL, NULL, NULL, NULL,NULL, &RunLoopSourcePerformRoutine};
      mProcessMessageLoopSource = CFRunLoopSourceCreate(NULL, 1, &context);
      ZS_ASSERT((NULL != mProcessMessageLoopSource) && CFRunLoopSourceIsValid(mProcessMessageLoopSource));

      CFRunLoopAddSource(mRunLoop, mProcessMessageLoopSource, kCFRunLoopDefaultMode);

      ZS_ASSERT(CFRunLoopContainsSource(mRunLoop, mProcessMessageLoopSource, kCFRunLoopDefaultMode));

      CFRunLoopSourceContext contextMoreMessages = {0, this, NULL, NULL, NULL, NULL, NULL, NULL,NULL, &MoreMessagesLoopSourcePerformRoutine};
      mMoreMessagesLoopSource = CFRunLoopSourceCreate(NULL, 1, &contextMoreMessages);

      ZS_ASSERT((NULL != mMoreMessagesLoopSource) && CFRunLoopSourceIsValid(mMoreMessagesLoopSource));
      CFRunLoopAddSource(mRunLoop, mMoreMessagesLoopSource, kCFRunLoopDefaultMode);

      ZS_ASSERT(CFRunLoopContainsSource(mRunLoop, mMoreMessagesLoopSource, kCFRunLoopDefaultMode));
    }

    //-------------------------------------------------------------------------
    MessageQueueThreadUsingMainThreadMessageQueueForApple::MessageQueueThreadUsingMainThreadMessageQueueForApple() noexcept :
      mProcessMessageLoopSource(NULL),
      mMoreMessagesLoopSource(NULL)
    {
    }

    //-------------------------------------------------------------------------
    MessageQueueThreadUsingMainThreadMessageQueueForApple::~MessageQueueThreadUsingMainThreadMessageQueueForApple() noexcept
    {
      waitForShutdown();

      if (NULL != mProcessMessageLoopSource)
      {
        CFRunLoopSourceInvalidate(mProcessMessageLoopSource);
      }

      if (NULL != mMoreMessagesLoopSource)
      {
        CFRunLoopSourceInvalidate(mMoreMessagesLoopSource);
      }
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingMainThreadMessageQueueForApple::process() noexcept
    {
      //process only one message in que
      mQueue->processOnlyOneMessage();

      //if there are more messages to process signal it
      if (this->getTotalUnprocessedMessages() > 0)
      {
        ZS_ASSERT((NULL != mMoreMessagesLoopSource) && CFRunLoopSourceIsValid(mMoreMessagesLoopSource));

        CFRunLoopSourceSignal(mMoreMessagesLoopSource);
        CFRunLoopWakeUp(mRunLoop);
      }

    }

    //-------------------------------------------------------------------------
    bool MessageQueueThreadUsingMainThreadMessageQueueForApple::isCurrentThread() const noexcept
    {
      return CFRunLoopGetMain() == CFRunLoopGetCurrent();
    }

    //-------------------------------------------------------------------------
    zsLib::Log::Params MessageQueueThreadUsingMainThreadMessageQueueForApple::slog(const char *message) noexcept
    {
      return zsLib::Log::Params(message, "MessageQueueThreadUsingMainThreadMessageQueueForApple");
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingMainThreadMessageQueueForApple::post(IMessageQueueMessageUniPtr message) noexcept(false)
    {
      if (mIsShutdown) {
        ZS_THROW_CUSTOM(Exceptions::MessageQueueAlreadyDeleted, "message posted to message queue after message queue was deleted.");
      }
      mQueue->post(std::move(message));
    }

    //-------------------------------------------------------------------------
    IMessageQueue::size_type MessageQueueThreadUsingMainThreadMessageQueueForApple::getTotalUnprocessedMessages() const noexcept
    {
      AutoLock lock(mLock);
      return mQueue->getTotalUnprocessedMessages();
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingMainThreadMessageQueueForApple::notifyMessagePosted() noexcept
    {
      AutoLock lock(mLock);
      ZS_ASSERT((NULL != mProcessMessageLoopSource) && CFRunLoopSourceIsValid(mProcessMessageLoopSource));

      CFRunLoopSourceSignal(mProcessMessageLoopSource);
      CFRunLoopWakeUp(mRunLoop);
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingMainThreadMessageQueueForApple::waitForShutdown() noexcept
    {
      AutoLock lock(mLock);
      mQueue.reset();

      if (NULL != mProcessMessageLoopSource)
      {
        CFRunLoopSourceInvalidate(mProcessMessageLoopSource);
        mProcessMessageLoopSource = NULL;

      }

      if (NULL != mMoreMessagesLoopSource)
      {
        CFRunLoopSourceInvalidate(mMoreMessagesLoopSource);
        mMoreMessagesLoopSource = NULL;
      }

      mIsShutdown = true;
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingMainThreadMessageQueueForApple::setThreadPriority(ThreadPriorities threadPriority) noexcept
    {
      // no-op
    }

    //-------------------------------------------------------------------------
    void MessageQueueThreadUsingMainThreadMessageQueueForApple::processMessagesFromThread() noexcept
    {
      mQueue->process();
    }

    //-------------------------------------------------------------------------
    void RunLoopSourcePerformRoutine(void *info) noexcept
    {
      MessageQueueThreadUsingMainThreadMessageQueueForApple* obj = (MessageQueueThreadUsingMainThreadMessageQueueForApple*)info;
      obj->process();
    }

    //-------------------------------------------------------------------------
    void MoreMessagesLoopSourcePerformRoutine(void *info) noexcept
    {
      MessageQueueThreadUsingMainThreadMessageQueueForApple* obj = (MessageQueueThreadUsingMainThreadMessageQueueForApple*)info;
      obj->notifyMessagePosted();
    }
  }
}

#endif //__APPLE__

