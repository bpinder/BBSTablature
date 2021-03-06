/*
  ==============================================================================

   This file is part of the JUCE library - "Jules' Utility Class Extensions"
   Copyright 2004-11 by Raw Material Software Ltd.

  ------------------------------------------------------------------------------

   JUCE can be redistributed and/or modified under the terms of the GNU General
   Public License (Version 2), as published by the Free Software Foundation.
   A copy of the license is included in the JUCE distribution, or can be found
   online at www.gnu.org/licenses.

   JUCE is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
   A PARTICULAR PURPOSE.  See the GNU General Public License for more details.

  ------------------------------------------------------------------------------

   To release a closed-source product which uses JUCE, commercial licenses are
   available: visit www.rawmaterialsoftware.com/juce for more information.

  ==============================================================================
*/

class MessageManager::QuitMessage   : public MessageManager::MessageBase
{
public:
    QuitMessage() {}

    void messageCallback()
    {
        if (MessageManager* const mm = MessageManager::instance)
            mm->quitMessageReceived = true;
    }

    JUCE_DECLARE_NON_COPYABLE (QuitMessage)
};

//==============================================================================
MessageManager::MessageManager() noexcept
  : quitMessagePosted (false),
    quitMessageReceived (false),
    messageThreadId (Thread::getCurrentThreadId()),
    threadWithLock (0)
{
    if (JUCEApplicationBase::isStandaloneApp())
        Thread::setCurrentThreadName ("Juce Message Thread");
}

MessageManager::~MessageManager() noexcept
{
    broadcaster = nullptr;

    doPlatformSpecificShutdown();

    jassert (instance == this);
    instance = nullptr;  // do this last in case this instance is still needed by doPlatformSpecificShutdown()
}

MessageManager* MessageManager::instance = nullptr;

MessageManager* MessageManager::getInstance()
{
    if (instance == nullptr)
    {
        instance = new MessageManager();
        doPlatformSpecificInitialisation();
    }

    return instance;
}

inline MessageManager* MessageManager::getInstanceWithoutCreating() noexcept
{
    return instance;
}

void MessageManager::deleteInstance()
{
    deleteAndZero (instance);
}

//==============================================================================
void MessageManager::MessageBase::post()
{
    MessageManager* const mm = MessageManager::instance;

    if (mm == nullptr || mm->quitMessagePosted || ! postMessageToSystemQueue (this))
        Ptr deleter (this); // (this will delete messages that were just created with a 0 ref count)
}

//==============================================================================
#if JUCE_MODAL_LOOPS_PERMITTED && ! (JUCE_MAC || JUCE_IOS)
void MessageManager::runDispatchLoop()
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    runDispatchLoopUntil (-1);
}

void MessageManager::stopDispatchLoop()
{
    (new QuitMessage())->post();
    quitMessagePosted = true;
}

bool MessageManager::runDispatchLoopUntil (int millisecondsToRunFor)
{
    jassert (isThisTheMessageThread()); // must only be called by the message thread

    const int64 endTime = Time::currentTimeMillis() + millisecondsToRunFor;

    while ((millisecondsToRunFor < 0 || endTime > Time::currentTimeMillis())
            && ! quitMessageReceived)
    {
        JUCE_TRY
        {
            if (! dispatchNextMessageOnSystemQueue (millisecondsToRunFor >= 0))
            {
                const int msToWait = (int) (endTime - Time::currentTimeMillis());

                if (msToWait > 0)
                    Thread::sleep (jmin (5, msToWait));
            }
        }
        JUCE_CATCH_EXCEPTION
    }

    return ! quitMessageReceived;
}

#endif

//==============================================================================
class AsyncFunctionCallback   : public MessageManager::MessageBase
{
public:
    AsyncFunctionCallback (MessageCallbackFunction* const f, void* const param)
        : result (nullptr), func (f), parameter (param)
    {}

    void messageCallback()
    {
        result = (*func) (parameter);
        finished.signal();
    }

    WaitableEvent finished;
    void* volatile result;

private:
    MessageCallbackFunction* const func;
    void* const parameter;

    JUCE_DECLARE_NON_COPYABLE (AsyncFunctionCallback)
};

void* MessageManager::callFunctionOnMessageThread (MessageCallbackFunction* const func, void* const parameter)
{
    if (isThisTheMessageThread())
        return func (parameter);

    // If this thread has the message manager locked, then this will deadlock!
    jassert (! currentThreadHasLockedMessageManager());

    const ReferenceCountedObjectPtr<AsyncFunctionCallback> message (new AsyncFunctionCallback (func, parameter));
    message->post();
    message->finished.wait();
    return message->result;
}

//==============================================================================
void MessageManager::deliverBroadcastMessage (const String& value)
{
    if (broadcaster != nullptr)
        broadcaster->sendActionMessage (value);
}

void MessageManager::registerBroadcastListener (ActionListener* const listener)
{
    if (broadcaster == nullptr)
        broadcaster = new ActionBroadcaster();

    broadcaster->addActionListener (listener);
}

void MessageManager::deregisterBroadcastListener (ActionListener* const listener)
{
    if (broadcaster != nullptr)
        broadcaster->removeActionListener (listener);
}

//==============================================================================
bool MessageManager::isThisTheMessageThread() const noexcept
{
    return Thread::getCurrentThreadId() == messageThreadId;
}

void MessageManager::setCurrentThreadAsMessageThread()
{
    const Thread::ThreadID thisThread = Thread::getCurrentThreadId();

    if (messageThreadId != thisThread)
    {
        messageThreadId = thisThread;

        // This is needed on windows to make sure the message window is created by this thread
        doPlatformSpecificShutdown();
        doPlatformSpecificInitialisation();
    }
}

bool MessageManager::currentThreadHasLockedMessageManager() const noexcept
{
    const Thread::ThreadID thisThread = Thread::getCurrentThreadId();
    return thisThread == messageThreadId || thisThread == threadWithLock;
}

//==============================================================================
//==============================================================================
/*  The only safe way to lock the message thread while another thread does
    some work is by posting a special message, whose purpose is to tie up the event
    loop until the other thread has finished its business.

    Any other approach can get horribly deadlocked if the OS uses its own hidden locks which
    get locked before making an event callback, because if the same OS lock gets indirectly
    accessed from another thread inside a MM lock, you're screwed. (this is exactly what happens
    in Cocoa).
*/
class MessageManagerLock::BlockingMessage   : public MessageManager::MessageBase
{
public:
    BlockingMessage() noexcept {}

    void messageCallback()
    {
        lockedEvent.signal();
        releaseEvent.wait();
    }

    WaitableEvent lockedEvent, releaseEvent;

    JUCE_DECLARE_NON_COPYABLE (BlockingMessage)
};

//==============================================================================
MessageManagerLock::MessageManagerLock (Thread* const threadToCheck)
    : blockingMessage(), locked (attemptLock (threadToCheck, nullptr))
{
}

MessageManagerLock::MessageManagerLock (ThreadPoolJob* const jobToCheckForExitSignal)
    : blockingMessage(), locked (attemptLock (nullptr, jobToCheckForExitSignal))
{
}

bool MessageManagerLock::attemptLock (Thread* const threadToCheck, ThreadPoolJob* const job)
{
    MessageManager* const mm = MessageManager::instance;

    if (mm == nullptr)
        return false;

    if (mm->currentThreadHasLockedMessageManager())
        return true;

    if (threadToCheck == nullptr && job == nullptr)
    {
        mm->lockingLock.enter();
    }
    else
    {
        while (! mm->lockingLock.tryEnter())
        {
            if ((threadToCheck != nullptr && threadToCheck->threadShouldExit())
                  || (job != nullptr && job->shouldExit()))
                return false;

            Thread::yield();
        }
    }

    blockingMessage = new BlockingMessage();
    blockingMessage->post();

    while (! blockingMessage->lockedEvent.wait (20))
    {
        if ((threadToCheck != nullptr && threadToCheck->threadShouldExit())
              || (job != nullptr && job->shouldExit()))
        {
            blockingMessage->releaseEvent.signal();
            blockingMessage = nullptr;
            mm->lockingLock.exit();
            return false;
        }
    }

    jassert (mm->threadWithLock == 0);

    mm->threadWithLock = Thread::getCurrentThreadId();
    return true;
}

MessageManagerLock::~MessageManagerLock() noexcept
{
    if (blockingMessage != nullptr)
    {
        MessageManager* const mm = MessageManager::instance;

        jassert (mm == nullptr || mm->currentThreadHasLockedMessageManager());

        blockingMessage->releaseEvent.signal();
        blockingMessage = nullptr;

        if (mm != nullptr)
        {
            mm->threadWithLock = 0;
            mm->lockingLock.exit();
        }
    }
}

//==============================================================================
JUCE_API void JUCE_CALLTYPE initialiseJuce_GUI();
JUCE_API void JUCE_CALLTYPE initialiseJuce_GUI()
{
    JUCE_AUTORELEASEPOOL
    MessageManager::getInstance();
}

JUCE_API void JUCE_CALLTYPE shutdownJuce_GUI();
JUCE_API void JUCE_CALLTYPE shutdownJuce_GUI()
{
    JUCE_AUTORELEASEPOOL
    DeletedAtShutdown::deleteAll();
    MessageManager::deleteInstance();
}
