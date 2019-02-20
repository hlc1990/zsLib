/*

 Copyright (c) 2016, Robin Raymond
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

#include <zsLib/internal/zsLib_MessageQueueManager.h>
#include <zsLib/IMessageQueueThreadPool.h>
#include <zsLib/IHelper.h>
#include <zsLib/ISettings.h>

#include <zsLib/Log.h>
#include <zsLib/XML.h>

#define ZSLIB_MESSAGE_QUEUE_MANAGER_RESERVED_GUI_THREAD_NAME "c745461ccd5bfd8427beeda5f952dc68fb09668a_zsLib.guiThread"

namespace zsLib { ZS_DECLARE_SUBSYSTEM(zslib) }

namespace zsLib
{
  namespace internal
  {
    ZS_DECLARE_USING_PTR(zsLib::XML, Element);
    ZS_DECLARE_USING_PTR(zsLib, IMessageQueueThread);

    ZS_DECLARE_CLASS_PTR(MessageQueueManagerSettingsDefaults);

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // (helpers)
    //

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // SocketMonitorSettingsDefaults
    //

    class MessageQueueManagerSettingsDefaults : public ISettingsApplyDefaultsDelegate
    {
    public:
      //-----------------------------------------------------------------------
      ~MessageQueueManagerSettingsDefaults() noexcept
      {
        ISettings::removeDefaults(*this);
      }

      //-----------------------------------------------------------------------
      static MessageQueueManagerSettingsDefaultsPtr singleton() noexcept
      {
        static SingletonLazySharedPtr<MessageQueueManagerSettingsDefaults> singleton(create());
        return singleton.singleton();
      }

      //-----------------------------------------------------------------------
      static MessageQueueManagerSettingsDefaultsPtr create() noexcept
      {
        auto pThis(make_shared<MessageQueueManagerSettingsDefaults>());
        ISettings::installDefaults(pThis);
        return pThis;
      }

      //-----------------------------------------------------------------------
      void notifySettingsApplyDefaults() noexcept override
      {
        ISettings::setBool(ZSLIB_SETTING_MESSAGE_QUEUE_MANAGER_PROCESS_APPLICATION_MESSAGE_QUEUE_ON_QUIT, false);
      }
    };

    //-------------------------------------------------------------------------
    void installMessageQueueManagerSettingsDefaults() noexcept
    {
      MessageQueueManagerSettingsDefaults::singleton();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MessageQueueManager
    //

    //-------------------------------------------------------------------------
    MessageQueueManager::MessageQueueManager(const make_private &) noexcept :
      mPending(0),
      mProcessApplicationQueueOnShutdown(ISettings::getBool(ZSLIB_SETTING_MESSAGE_QUEUE_MANAGER_PROCESS_APPLICATION_MESSAGE_QUEUE_ON_QUIT))
    {
      ZS_LOG_BASIC(log("created"))
    }

    //-------------------------------------------------------------------------
    void MessageQueueManager::init() noexcept
    {
      // AutoRecursiveLock lock(mLock);
    }

    //-------------------------------------------------------------------------
    MessageQueueManagerPtr MessageQueueManager::create() noexcept
    {
      MessageQueueManagerPtr pThis(make_shared<MessageQueueManager>(make_private{}));
      pThis->mThisWeak = pThis;
      pThis->init();
      return pThis;
    }

    //-------------------------------------------------------------------------
    MessageQueueManagerPtr MessageQueueManager::singleton() noexcept
    {
      AutoRecursiveLock lock(*IHelper::getGlobalLock());
      static SingletonLazySharedPtr<MessageQueueManager> singleton(create());
      MessageQueueManagerPtr result = singleton.singleton();

      static zsLib::SingletonManager::Register registerSingleton("org.zsLib.MessageQueueManager", result);

      if (!result) {
        ZS_LOG_WARNING(Detail, slog("singleton gone"))
      }

      return result;
    }

    //-------------------------------------------------------------------------
    MessageQueueManager::~MessageQueueManager() noexcept
    {
      ZS_LOG_BASIC(log("destroyed"))
      mThisWeak.reset();
      cancel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MessageQueueManager => IMessageQueueManager
    //

    //-------------------------------------------------------------------------
    IMessageQueuePtr MessageQueueManager::getMessageQueueForGUIThread() noexcept
    {
      return getMessageQueue(ZSLIB_MESSAGE_QUEUE_MANAGER_RESERVED_GUI_THREAD_NAME);
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr MessageQueueManager::getMessageQueue(const char *assignedQueueName) noexcept
    {
      String name(assignedQueueName);

      AutoRecursiveLock lock(mLock);

      // scope: check thread queues
      {
        MessageQueueMap::iterator found = mQueues.find(name);
        if (found != mQueues.end()) {
          ZS_LOG_TRACE(log("re-using existing message queue with name") + ZS_PARAM("name", name))
          return (*found).second;
        }
      }

      {
        auto found = mPools.find(name);
        if (found != mPools.end()) {
          return ((*found).second.first)->createQueue();
        }
      }

      IMessageQueuePtr queue;

      // creating new thread
      if (ZSLIB_MESSAGE_QUEUE_MANAGER_RESERVED_GUI_THREAD_NAME == name) {
        ZS_LOG_TRACE(log("creating GUI thread"));
        queue = IMessageQueueThread::singletonUsingCurrentGUIThreadsMessageQueue();
      } else {

        ThreadPriorities priority = zsLib::ThreadPriority_Normal;

        ThreadPriorityMap::const_iterator foundPriority = mThreadPriorities.find(name);
        if (foundPriority != mThreadPriorities.end()) {
          priority = (*foundPriority).second;
        }

        ZS_LOG_TRACE(log("creating thread queue") + ZS_PARAM("name", name) + ZS_PARAM("priority", zsLib::toString(priority)))

        queue = IMessageQueueThread::createBasic(name, priority);
      }

      mQueues[name] = queue;
      return queue;
    }

    //-------------------------------------------------------------------------
    IMessageQueuePtr MessageQueueManager::getThreadPoolQueue(
                                                             const char *assignedThreadPoolQueueName,
                                                             size_t minThreadsRequired
                                                             ) noexcept
    {
      String poolName(assignedThreadPoolQueueName);

      AutoRecursiveLock lock(mLock);

      ThreadPriorities priority = zsLib::ThreadPriority_Normal;

      ThreadPriorityMap::const_iterator foundPriority = mThreadPriorities.find(poolName);
      if (foundPriority != mThreadPriorities.end()) {
        priority = (*foundPriority).second;
      }

      if (ZSLIB_MESSAGE_QUEUE_MANAGER_RESERVED_GUI_THREAD_NAME == poolName) {
        ZS_LOG_TRACE(log("creating GUI thread"));
        return IMessageQueueThread::singletonUsingCurrentGUIThreadsMessageQueue();
      }      

      {
        auto found = mQueues.find(poolName);
        if (found != mQueues.end()) {
          ZS_LOG_ERROR(Basic, log("re-using existing thread message queue with name") + ZS_PARAM("name", poolName));
          return (*found).second;
        }
      }

      IMessageQueueThreadPoolPtr pool;
      size_t totalThreadsCreated = 0;

      {
        auto found = mPools.find(poolName);
        if (found != mPools.end()) {
          pool = (*found).second.first;
          totalThreadsCreated = (*found).second.second;
        }
      }

      if (!pool) {
        ZS_LOG_TRACE(log("creating thread pool") + ZS_PARAM("name", poolName));
        pool = IMessageQueueThreadPool::create();
      }

      while (totalThreadsCreated < minThreadsRequired) {
        ++totalThreadsCreated;
        ZS_LOG_TRACE(log("creating pool thread") + ZS_PARAM("poolName", poolName + "." + string(totalThreadsCreated)) + ZS_PARAM("priority", zsLib::toString(priority)));
        pool->createThread((poolName + "." + string(totalThreadsCreated)).c_str(), priority);
      }

      mPools[poolName] = MessageQueueThreadPoolPair(pool, totalThreadsCreated);

      IMessageQueuePtr queue = pool->createQueue();
      return queue;
    }

    //-------------------------------------------------------------------------
    void MessageQueueManager::registerMessageQueueThreadPriority(
                                                                 const char *assignedQueueName,
                                                                 ThreadPriorities priority
                                                                 ) noexcept
    {
      ZS_DECLARE_TYPEDEF_PTR(zsLib::IMessageQueueThread, IMessageQueueThread);

      AutoRecursiveLock lock(mLock);

      String name(assignedQueueName);
      mThreadPriorities[name] = priority;

      bool inUse = false;

      // scope: fix existing queue thread priorities
      {
        auto found = mQueues.find(name);
        if (found != mQueues.end()) {
          ZS_LOG_DEBUG(log("updating message queue thread") + ZS_PARAM("name", name) + ZS_PARAM("priority", zsLib::toString(priority)));

          IMessageQueuePtr queue = (*found).second;

          IMessageQueueThreadPtr thread = ZS_DYNAMIC_PTR_CAST(IMessageQueueThread, queue);
          if (thread) {
            thread->setThreadPriority(priority);
            inUse = true;
          } else {
            ZS_LOG_WARNING(Detail, log("found thread was not recognized as a message queue thread") + ZS_PARAM("name", name));
          }
        }
      }

      // scope: fix existing pool queue thrad priorities
      {
        auto found = mPools.find(name);
        if (found != mPools.end()) {
          ZS_LOG_DEBUG(log("updating message queue thread pool") + ZS_PARAM("name", name) + ZS_PARAM("priority", zsLib::toString(priority)));

          IMessageQueueThreadPoolPtr pool = (*found).second.first;

          pool->setThreadPriority(priority);
          inUse = true;
        }
      }

      if (!inUse) {
        ZS_LOG_DEBUG(log("message queue specified is not in use at yet") + ZS_PARAM("name", name) + ZS_PARAM("priority", zsLib::toString(priority)));
      }
    }

    //-------------------------------------------------------------------------
    size_t MessageQueueManager::getTotalUnprocessedMessages() const noexcept
    {
      AutoRecursiveLock lock(mLock);

      size_t result = 0;

      for (MessageQueueMap::const_iterator iter = mQueues.begin(); iter != mQueues.end(); ++iter) {
        const IMessageQueuePtr &queue = (*iter).second;
        result += queue->getTotalUnprocessedMessages();
      }

      for (auto iter = mPools.begin(); iter != mPools.end(); ++iter) {
        auto pool = (*iter).second.first;
        result += (pool->hasPendingMessages() ? 1 : 0);
      }

      return result;
    }

    //-------------------------------------------------------------------------
    void MessageQueueManager::shutdownAllQueues() noexcept
    {
      ZS_LOG_DETAIL(log("shutdown all queues called"))

      AutoRecursiveLock lock(mLock);
      mGracefulShutdownReference = mThisWeak.lock();

      onWake();
    }

    //-------------------------------------------------------------------------
    void MessageQueueManager::blockUntilDone() noexcept
    {
      bool processApplicationQueueOnShutdown = false;

      MessageQueueMap queues;
      MessageQueuePoolMap pools;

      {
        AutoRecursiveLock lock(mLock);
        processApplicationQueueOnShutdown = mProcessApplicationQueueOnShutdown;
        queues = mQueues;
        pools = mPools;
      }

      size_t totalRemaining = 0;

      do
      {
        totalRemaining = 0;

        for (MessageQueueMap::iterator iter = queues.begin(); iter != queues.end(); ++iter)
        {
          const MessageQueueName &name = (*iter).first;
          IMessageQueuePtr queue = (*iter).second;

          if (name == ZSLIB_MESSAGE_QUEUE_MANAGER_RESERVED_GUI_THREAD_NAME) {
            if (!processApplicationQueueOnShutdown)
              continue;

            IMessageQueueThreadPtr thread = ZS_DYNAMIC_PTR_CAST(IMessageQueueThread, queue);

            if (thread) {
              thread->processMessagesFromThread();
            }
          }

          totalRemaining += queue->getTotalUnprocessedMessages();
        }

        for (auto iter = pools.begin(); iter != pools.end(); ++iter) {
          auto pool = (*iter).second.first;
          if (pool->hasPendingMessages()) ++totalRemaining;
        }

        if (totalRemaining < 1) {
          break;
        }

        std::this_thread::yield();
      } while (totalRemaining > 0);
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MessageQueueManager => IWakeDelegate
    //

    //-------------------------------------------------------------------------
    void MessageQueueManager::onWake()
    {
      AutoRecursiveLock lock(mLock);

      if (!mGracefulShutdownReference) return;

      if (0 != mPending) {
        --mPending;
        if (0 != mPending) return;
      }

      if ((mQueues.size() < 1) &&
          (mPools.size() < 1)) {
        mGracefulShutdownReference.reset();
        return;
      }

      for (MessageQueueMap::iterator iter = mQueues.begin(); iter != mQueues.end(); ++iter)
      {
        IMessageQueuePtr queue = (*iter).second;

        size_t remaining = queue->getTotalUnprocessedMessages();
        if (0 == remaining) continue;

        ++mPending;

        IWakeDelegateProxy::create(queue, mThisWeak.lock())->onWake();
        std::this_thread::yield();
      }

      for (auto iter = mPools.begin(); iter != mPools.end(); ++iter) {
        auto pool = (*iter).second.first;

        bool hasPending = pool->hasPendingMessages();
        if (!hasPending) continue;

        ++mPending;

        IMessageQueuePtr tempQueue = pool->createQueue();
        IWakeDelegateProxy::create(tempQueue, mThisWeak.lock())->onWake();
        std::this_thread::yield();
      }

      if (0 != mPending) {
        mFinalCheck = false;
        return;
      }

      if (0 == mPending) {
        if (!mFinalCheck) {
          mFinalCheck = true;

          if (mQueues.size() > 0) {
            // perform one-time double check to truly make sure all queues are empty
            for (auto iter = mQueues.begin(); iter != mQueues.end(); ++iter) {
              const MessageQueueName &name = (*iter).first;
              IMessageQueuePtr queue = (*iter).second;

              if (name == ZSLIB_MESSAGE_QUEUE_MANAGER_RESERVED_GUI_THREAD_NAME)
                continue;

              ++mPending;
              IWakeDelegateProxy::create(queue, mThisWeak.lock())->onWake();
              std::this_thread::yield();
              return;
            }
          }

          if (mPools.size() > 0) {
            auto pool = (*(mPools.begin())).second.first;

            ++mPending;

            IMessageQueuePtr tempQueue = pool->createQueue();
            IWakeDelegateProxy::create(tempQueue, mThisWeak.lock())->onWake();
            std::this_thread::yield();
            return;
          }
        }
      }

      // all queue are empty
      mFinalCheckComplete = true;
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MessageQueueManager => IMessageQueueManagerForBackgrounding
    //

    //-------------------------------------------------------------------------
    void MessageQueueManager::notifySingletonCleanup() noexcept
    {
      shutdownAllQueues();
      blockUntilDone();
      while (true) {
        if (mFinalCheckComplete) break;
        std::this_thread::yield();
      }
      cancel();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //
    // MessageQueueManager => (internal)
    //

    //-------------------------------------------------------------------------
    MessageQueueManager::Params MessageQueueManager::log(const char *message) const noexcept
    {
      ElementPtr objectEl = Element::create("MessageQueueManager");
      IHelper::debugAppend(objectEl, "id", mID);
      return Params(message, objectEl);
    }

    //-------------------------------------------------------------------------
    MessageQueueManager::Params MessageQueueManager::slog(const char *message) noexcept
    {
      return Params(message, "MessageQueueManager");
    }

    //-------------------------------------------------------------------------
    MessageQueueManager::Params MessageQueueManager::debug(const char *message) const noexcept
    {
      return Params(message, toDebug());
    }

    //-------------------------------------------------------------------------
    ElementPtr MessageQueueManager::toDebug() const noexcept
    {
      AutoRecursiveLock lock(mLock);
      ElementPtr resultEl = Element::create("MessageQueueManager");

      IHelper::debugAppend(resultEl, "id", mID);
      IHelper::debugAppend(resultEl, "graceful reference", (bool)mGracefulShutdownReference);
      IHelper::debugAppend(resultEl, "final check", mFinalCheck);

      IHelper::debugAppend(resultEl, "total queues", mQueues.size());
      IHelper::debugAppend(resultEl, "total priorities", mThreadPriorities.size());

      IHelper::debugAppend(resultEl, "pools", mPools.size());

      IHelper::debugAppend(resultEl, "process application queue on shutdown", mProcessApplicationQueueOnShutdown);

      return resultEl;
    }

    //-------------------------------------------------------------------------
    void MessageQueueManager::cancel() noexcept
    {
      ZS_LOG_DEBUG(log("cancel called"))

      MessageQueueMap queues;
      MessageQueuePoolMap pools;

      {
        queues = mQueues;
        mQueues.clear();
      }

      // scope: clean out basic thread queues
      {
        for (MessageQueueMap::iterator iter_doNotUse = queues.begin(); iter_doNotUse != queues.end(); ++iter_doNotUse)
        {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          MessageQueueName name = (*current).first;
          IMessageQueuePtr queue = (*current).second;

          size_t totalMessagesLeft = queue->getTotalUnprocessedMessages();
          if (totalMessagesLeft > 0) {
            ZS_LOG_WARNING(Basic, log("unprocessed messages are still in the queue - did you check getTotalUnprocessedMessages() to make sure all queues are empty before quiting?"))
          }

          IMessageQueueThreadPtr threadQueue = ZS_DYNAMIC_PTR_CAST(IMessageQueueThread, queue);

          if (threadQueue) {
            threadQueue->waitForShutdown();
          }
          queues.erase(current);
        }
      }

      {
        AutoRecursiveLock lock(mLock);
        pools = mPools;

        mPools.clear();
      }

      // scope: clean out thread pool queues
      {
        for (auto iter_doNotUse = pools.begin(); iter_doNotUse != pools.end(); ) {
          auto current = iter_doNotUse;
          ++iter_doNotUse;

          auto pool = (*current).second.first;

          IMessageQueuePtr queue = pool->createQueue();

          size_t totalMessagesLeft = queue->getTotalUnprocessedMessages();
          if (totalMessagesLeft > 0) {
            ZS_LOG_WARNING(Basic, log("unprocessed messages are still in the queue - did you check getTotalUnprocessedMessages() to make sure all queues are empty before quiting?"))
          }

          pool->waitForShutdown();

          pools.erase(current);
        }
      }

      mGracefulShutdownReference.reset();
    }

    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------
    //-------------------------------------------------------------------------

  } // namespace internal

  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //---------------------------------------------------------------------------
  //
  // IMessageQueueManager
  //

  //---------------------------------------------------------------------------
  IMessageQueuePtr IMessageQueueManager::getMessageQueueForGUIThread() noexcept
  {
    internal::MessageQueueManagerPtr singleton = internal::MessageQueueManager::singleton();
    if (!singleton) return IMessageQueuePtr();
    return singleton->getMessageQueueForGUIThread();
  }

  //---------------------------------------------------------------------------
  IMessageQueuePtr IMessageQueueManager::getMessageQueue(const char *assignedQueueName) noexcept
  {
    internal::MessageQueueManagerPtr singleton = internal::MessageQueueManager::singleton();
    if (!singleton) return IMessageQueuePtr();
    return singleton->getMessageQueue(assignedQueueName);
  }

  //---------------------------------------------------------------------------
  IMessageQueuePtr IMessageQueueManager::getThreadPoolQueue(
                                                            const char *assignedThreadPoolQueueName,
                                                            size_t minThreadsRequired
                                                            ) noexcept
  {
    internal::MessageQueueManagerPtr singleton = internal::MessageQueueManager::singleton();
    if (!singleton) return IMessageQueuePtr();
    return singleton->getThreadPoolQueue(assignedThreadPoolQueueName, minThreadsRequired);
  }

  //---------------------------------------------------------------------------
  void IMessageQueueManager::registerMessageQueueThreadPriority(
                                                                const char *assignedQueueName,
                                                                ThreadPriorities priority
                                                                ) noexcept
  {
    internal::MessageQueueManagerPtr singleton = internal::MessageQueueManager::singleton();
    if (!singleton) return;
    singleton->registerMessageQueueThreadPriority(assignedQueueName, priority);
  }

  //---------------------------------------------------------------------------
  size_t IMessageQueueManager::getTotalUnprocessedMessages() noexcept
  {
    internal::MessageQueueManagerPtr singleton = internal::MessageQueueManager::singleton();
    if (!singleton) return 0;
    return singleton->getTotalUnprocessedMessages();
  }

  //---------------------------------------------------------------------------
  void IMessageQueueManager::shutdownAllQueues() noexcept
  {
    internal::MessageQueueManagerPtr singleton = internal::MessageQueueManager::singleton();
    if (!singleton) return;
    singleton->shutdownAllQueues();
  }

  //-------------------------------------------------------------------------
  void IMessageQueueManager::blockUntilDone() noexcept
  {
    internal::MessageQueueManagerPtr singleton = internal::MessageQueueManager::singleton();
    if (!singleton) return;
    singleton->blockUntilDone();
  }


} // namespace zsLib
