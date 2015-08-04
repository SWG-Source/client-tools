#ifndef INCLUDED_SerializedObjectBuffer_H
#define INCLUDED_SerializedObjectBuffer_H

#include <vector>

class UIBaseObject;

class SerializedObjectBuffer
{
public:

	typedef std::vector<char> Buffer;

	int           size()      const { return m_data.size(); }
	const Buffer &getBuffer() const { return m_data; }

	void setBuffer(int size, const char *data);

	// --------------------------------------

	void serialize(UIBaseObject &subTree);

	UIBaseObject *unserialize();

	// --------------------------------------

protected:


	bool _extractNextString(std::string &o_dest, Buffer::iterator &io_source, const Buffer::iterator &sourceEnd);

	Buffer m_data;
};

#endif
