/*
 * Copyright (C) 2010 Apple Inc. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY APPLE INC. AND ITS CONTRIBUTORS ``AS IS''
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL APPLE INC. OR ITS CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "WorkQueue.h"

#include <mach/mach_port.h>

void WorkQueue::executeWorkItem(void* item)
{
    WorkQueue* queue = static_cast<WorkQueue*>(dispatch_get_context(dispatch_get_current_queue()));
    std::auto_ptr<WorkItem> workItem(static_cast<WorkItem*>(item));
    
    {
        MutexLocker locker(queue->m_isValidMutex);
        if (!queue->m_isValid)
            return;
    }
    
    workItem->execute();
}

void WorkQueue::scheduleWork(std::auto_ptr<WorkItem> item)
{
    dispatch_async_f(m_dispatchQueue, item.release(), executeWorkItem);
}

class WorkQueue::EventSource {
public:
    EventSource(MachPortEventType eventType, dispatch_source_t dispatchSource, std::auto_ptr<WorkItem> workItem)
    : m_eventType(eventType)
    , m_dispatchSource(dispatchSource)
    , m_workItem(workItem)
    {
    }
    
    dispatch_source_t dispatchSource() const { return m_dispatchSource; }
    
    static void eventHandler(void* source) 
    {
        EventSource* eventSource = static_cast<EventSource*>(source);
        
        eventSource->m_workItem->execute();
    }
    
    static void cancelHandler(void* source)
    {
        EventSource* eventSource = static_cast<EventSource*>(source);
        
        mach_port_t machPort = dispatch_source_get_handle(eventSource->m_dispatchSource);
        
        switch (eventSource->m_eventType) {
        case MachPortDataAvailable:
            // Release our receive right.
            mach_port_mod_refs(mach_task_self(), machPort, MACH_PORT_RIGHT_RECEIVE, -1);
            break;
        case MachPortDeadNameNotification:
            // Release our send right.
            mach_port_deallocate(mach_task_self(), machPort);
            break;
        }
    }
    
    static void finalizeHandler(void* source)
    {
        EventSource* eventSource = static_cast<EventSource*>(source);
        
        delete eventSource;
    }
    
private:
    MachPortEventType m_eventType;
    
    // This is a weak reference, since m_dispatchSource references the event source.
    dispatch_source_t m_dispatchSource;
    
    std::auto_ptr<WorkItem> m_workItem;
};

void WorkQueue::registerMachPortEventHandler(mach_port_t machPort, MachPortEventType eventType, std::auto_ptr<WorkItem> workItem)
{
    dispatch_source_type_t sourceType = 0;
    switch (eventType) {
    case MachPortDataAvailable:
        sourceType = DISPATCH_SOURCE_TYPE_MACH_RECV;
        break;
    case MachPortDeadNameNotification:
        sourceType = DISPATCH_SOURCE_TYPE_MACH_SEND;
        break;
    }
    
    dispatch_source_t dispatchSource = dispatch_source_create(sourceType, machPort, 0, m_dispatchQueue);
    
    EventSource* eventSource = new EventSource(eventType, dispatchSource, workItem);
    dispatch_set_context(dispatchSource, eventSource);
    
    dispatch_source_set_event_handler_f(dispatchSource, &EventSource::eventHandler);
    dispatch_source_set_cancel_handler_f(dispatchSource, &EventSource::cancelHandler);
    dispatch_set_finalizer_f(dispatchSource, &EventSource::finalizeHandler);
    
    // Add the source to our set of sources.
    {
        MutexLocker locker(m_eventSourcesMutex);
        
        ASSERT(!m_eventSources.contains(machPort));
        
        m_eventSources.set(machPort, eventSource);
        
        // And start it!
        dispatch_resume(dispatchSource);
    }
}

void WorkQueue::unregisterMachPortEventHandler(mach_port_t machPort)
{
    ASSERT(machPort);
    
    MutexLocker locker(m_eventSourcesMutex);
    
    HashMap<mach_port_t, EventSource*>::iterator it = m_eventSources.find(machPort);
    ASSERT(it != m_eventSources.end());
    
    ASSERT(m_eventSources.contains(machPort));

    EventSource* eventSource = it->second;
    // Cancel and release the source. It will be deleted in its finalize handler.
    dispatch_source_cancel(eventSource->dispatchSource());
    dispatch_release(eventSource->dispatchSource());

    m_eventSources.remove(it);    
}

void WorkQueue::platformInitialize(const char* name)
{
    m_dispatchQueue = dispatch_queue_create(name, 0);
    dispatch_set_context(m_dispatchQueue, this);
}

void WorkQueue::platformInvalidate()
{
#if !ASSERT_DISABLED
    MutexLocker locker(m_eventSourcesMutex);
    ASSERT(m_eventSources.isEmpty());
#endif
}
