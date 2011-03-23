// mqueue.cpp - written and placed in the public domain by Wei Dai

#include "pch.h"
#include "mqueue.h"

NAMESPACE_BEGIN(CryptoPP)

MessageQueue::MessageQueue(unsigned int nodeSize)
	: m_queue(nodeSize), m_lengths(1, 0)
{
}

bool MessageQueue::RetrieveNextMessage()
{
	if (NumberOfMessages() > 0 && !AnyRetrievable())
	{
		m_lengths.pop_front();
		return true;
	}
	else
		return false;
}

unsigned int MessageQueue::CopyMessagesTo(BufferedTransformation &target, unsigned int count) const
{
	ByteQueue::Walker walker(m_queue);
	std::deque<unsigned long>::const_iterator it = m_lengths.begin();
	unsigned int i;
	for (i=0; i<count && it != --m_lengths.end(); ++i, ++it)
	{
		walker.TransferTo(target, *it);
		if (GetAutoSignalPropagation())
			target.MessageEnd(GetAutoSignalPropagation()-1);
	}
	return i;
}

void MessageQueue::swap(MessageQueue &rhs)
{
	m_queue.swap(rhs.m_queue);
	m_lengths.swap(rhs.m_lengths);
}

NAMESPACE_END
