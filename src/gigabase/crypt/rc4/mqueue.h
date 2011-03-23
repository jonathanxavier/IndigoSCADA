#ifndef CRYPTOPP_MQUEUE_H
#define CRYPTOPP_MQUEUE_H

#include "queue.h"
#include "filters.h"
#include <deque>

NAMESPACE_BEGIN(CryptoPP)

//! Message Queue
class MessageQueue : public BufferedTransformationWithAutoSignal
{
public:
	MessageQueue(unsigned int nodeSize=256);

	void Put(byte inByte)
		{m_queue.Put(inByte); m_lengths.back()++;}
	void Put(const byte *inString, unsigned int length)
		{m_queue.Put(inString, length); m_lengths.back()+=length;}

	unsigned long MaxRetrievable() const
		{return m_lengths.front();}
	bool AnyRetrievable() const
		{return m_lengths.front() > 0;}

	unsigned long TransferTo(BufferedTransformation &target, unsigned long transferMax=ULONG_MAX)
		{return Got(m_queue.TransferTo(target, STDMIN(MaxRetrievable(), transferMax)));}
	unsigned long CopyTo(BufferedTransformation &target, unsigned long copyMax=ULONG_MAX) const
		{return m_queue.CopyTo(target, STDMIN(MaxRetrievable(), copyMax));}

	void MessageEnd(int=-1)
		{m_lengths.push_back(0);}

	unsigned long TotalBytesRetrievable() const
		{return m_queue.MaxRetrievable();}
	unsigned int NumberOfMessages() const
		{return m_lengths.size()-1;}
	bool RetrieveNextMessage();

	unsigned int CopyMessagesTo(BufferedTransformation &target, unsigned int count=UINT_MAX) const;

	void swap(MessageQueue &rhs);

private:
	unsigned long Got(unsigned long length)
		{assert(m_lengths.front() >= length); m_lengths.front() -= length; return length;}

	ByteQueue m_queue;
	std::deque<unsigned long> m_lengths;
};

NAMESPACE_END

NAMESPACE_BEGIN(std)
template<> inline void swap(CryptoPP::MessageQueue &a, CryptoPP::MessageQueue &b)
{
	a.swap(b);
}
NAMESPACE_END

#endif
