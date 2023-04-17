/////////////////////////////////////////////////////////////////
// ����ȭ ����										������
//
// �����͸� �޸𸮿� ��������
// ���ϴ� ������� �ױ����� �뵵��
// ���� ť ������ �����̴�.
/////////////////////////////////////////////////////////////////

#pragma once
#ifndef __SERIALIZATION_BUFFER_H__
#define	__SERIALIZATION_BUFFER_H__
class SerializationBuffer
{
public:
	enum eCapacity
	{
		DEFAULT = 2048
	};

	SerializationBuffer();
	SerializationBuffer(int capacity);
	virtual ~SerializationBuffer();

	int GetBufferSize();
	int GetUseSize();
	int GetFreeSize();
	int Enqueue(const char* buffer, int size);
	int Dequeue(char* const buffer, int size);
	int MoveRear(int size);
	int MoveFront(int size);
	void ClearBuffer(void);
	char* GetRearBufferPtr(void) const;
	char* GetFrontBufferPtr(void) const;

	SerializationBuffer& operator<<(unsigned char data);
	SerializationBuffer& operator<<(char data);

	SerializationBuffer& operator<<(unsigned short data);
	SerializationBuffer& operator<<(short data);

	SerializationBuffer& operator<<(unsigned int data);
	SerializationBuffer& operator<<(int data);

	SerializationBuffer& operator<<(unsigned long data);
	SerializationBuffer& operator<<(long data);

	SerializationBuffer& operator<<(unsigned __int64 data);
	SerializationBuffer& operator<<(__int64 data);

	SerializationBuffer& operator<<(float data);
	SerializationBuffer& operator<<(double data);
	SerializationBuffer& operator<<(long double data);

	SerializationBuffer& operator>>(unsigned char& data);
	SerializationBuffer& operator>>(char& data);

	SerializationBuffer& operator>>(unsigned short& data);
	SerializationBuffer& operator>>(short& data);

	SerializationBuffer& operator>>(unsigned int& data);
	SerializationBuffer& operator>>(int& data);

	SerializationBuffer& operator>>(unsigned long& data);
	SerializationBuffer& operator>>(long& data);

	SerializationBuffer& operator>>(unsigned __int64& data);
	SerializationBuffer& operator>>(__int64& data);

	SerializationBuffer& operator>>(float& data);
	SerializationBuffer& operator>>(double& data);
	SerializationBuffer& operator>>(long double& data);

protected:
	char* __internalBuffer;
	int __capacity;
	int __useSize;
	int __freeSize;
	int __queueFrontIndex;
	int __queueRearIndex;
};

#endif // !__SERIALIZATION_BUFFER_H__