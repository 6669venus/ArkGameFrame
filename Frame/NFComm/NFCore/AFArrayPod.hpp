#pragma once

#include <stdint.h>

class ArrayPodAlloc
{
public:
    ArrayPodAlloc() {}
    ~ArrayPodAlloc() {}

    void* Alloc(size_t size)
    {
        return new char[size];
    }

    void Free(void* ptr, size_t size)
    {
        delete[] (char*)ptr;
    }

    void Swap(ArrayPodAlloc& src)
    {

    }
};

template<typename TYPE, size_t SIZE, typename ALLOC = ArrayPodAlloc>
class ArraryPod
{
public:
    using self_t = ArraryPod<TYPE, SIZE, ALLOC>;

public:
    ArraryPod()
    {
        mpData = mxStack;
        mnCapacity = SIZE;
        mnSize = 0;
    }

    ArraryPod(const self_t& src)
    {
        mnSize = src.mnSize;
        if (mnSize <= SIZE)
        {
            mpData = mxStack;
            mnCapacity = SIZE;
        }
        else
        {
            mnCapacity = src.mnCapacity;
            mpData = (TYPE*)mxAlloc.Alloc(mnCapacity * sizeof(TYPE));
        }

        memcpy(mpData, src.mpData, mnSize * sizeof(TYPE));
    }

    ~ArraryPod()
    {
        if (mnCapacity > SIZE)
        {
            mxAlloc.Free(mpData, mnCapacity * sizeof(TYPE));
        }
    }

    self_t& operator=(const self_t& rhs)
    {
        self_t tmp(rhs);
        Swap(tmp);
        return *this;
    }

    void swap(self_t& src)
    {
        size_t tmp_size = src.mnSize;
        size_t tmp_capacity = src.mnCapacity;
        TYPE* tmp_data = src.mpData;
        TYPE tmp_stack[SIZE];

        if (tmp_capacity <= SIZE)
        {
            memcpy(tmp_stack, src.mxStack, tmp_size * sizeof(TYPE));
        }

        src.mnSize = this->mnSize;
        src.mnCapacity = this->mnCapacity;

        if (this->mnCapacity <= SIZE)
        {
            memcpy(src.mxStack, this->mxStack, mnSize * sizeof(TYPE));
            src.mpData = src.mxStack;
        }
        else
        {
            src.mpData = this->mpData;
        }
        //////////////////////////////////////////////////////////////////////////
        this->mnSize = tmp_size;
        this->mnCapacity = tmp_capacity;

        if (tmp_capacity <= SIZE)
        {
            memcpy(this->mxStack, tmp_stack, tmp_size * sizeof(TYPE));
            this->mpData = this->mxStack;
        }
        else
        {
            this->mpData = tmp_data;
        }

        this->mxAlloc.Swap(src.mxAlloc);
    }

    bool empty()
    {
        return (mnSize == 0);
    }

    size_t size() const
    {
        return mnSize;
    }

    const TYPE* data() const
    {
        return mpData;
    }

    void push_back(const TYPE& data)
    {
        if (mnSize == mnCapacity)
        {
            size_t new_size = mnSize * 2;
            TYPE* p = (TYPE*)mxAlloc.Alloc(new_size * sizeof(TYPE));
            memcpy(p, mpData, mnSize * sizeof(TYPE));

            if (mnCapacity > SIZE)
            {
                mxAlloc.Free(mpData, mnCapacity * sizeof(TYPE));
            }

            mpData = p;
            mnCapacity = new_size;
        }

        mpData[mnSize++] = data;
    }

    void pop_back()
    {
        Assert(mnSize > 0);
        --mnSize;
    }

    TYPE& back()
    {
        Assert(mnSize > 0);
        return mpData[mnSize - 1];
    }

    const TYPE& back() const
    {
        Assert(mnSize > 0);
        return mpData[mnSize - 1];
    }

    TYPE& operator[](size_t index)
    {
        Assert(mnSize > 0);
        return mpData[index];
    }

    //Ԥ����
    void reserve(size_t size)
    {
        if (size > mnCapacity)
        {
            TYPE* p = (TYPE*)mxAlloc.Alloc(size * sizeof(TYPE));
            memcpy(p, mpData, mnSize * sizeof(TYPE));

            if (mnCapacity > SIZE)
            {
                mxAlloc.Free(mpData, mnCapacity * sizeof(TYPE));
            }

            mpData = p;
            mnCapacity = size;
        }
    }

    void resize(size_t size)
    {
        if (size > mnCapacity)
        {
            //��������������������������������Ͱ���size������
            size_t new_size = mnCapacity * 2;
            if (new_size < size)
            {
                new_size = size;
            }

            TYPE* p = (TYPE*)mxAlloc.Alloc(new_size * sizeof(TYPE));
            memcpy(p, mpData, mnSize * sizeof(TYPE)); //��ԭ��������mnSize���ݸ�ֵ���µĿռ�
            if (mnCapacity > SIZE)
            {
                mxAlloc.Free(mpData, mnCapacity * sizeof(TYPE));
            }

            mpData = p;
            mnCapacity = new_size; //�����ĳ��µ�
        }

        mnSize = size;
    }

    void resize(size_t size, const TYPE& value)
    {
        if (size > mnCapacity)
        {
            //��������������������������������Ͱ���size������
            size_t new_size = mnCapacity * 2;
            if (new_size < size)
            {
                new_size = size;
            }

            TYPE* p = (TYPE*)mxAlloc.Alloc(new_size * sizeof(TYPE));
            memcpy(p, mpData, mnSize * sizeof(TYPE)); //��ԭ��������mnSize���ݸ�ֵ���µĿռ�
            if (mnCapacity > SIZE)
            {
                mxAlloc.Free(mpData, mnCapacity * sizeof(TYPE));
            }

            mpData = p;
            mnCapacity = new_size; //�����ĳ��µ�
        }

        if (size > mnSize)
        {
            for (size_t i = mnSize; i < size; ++i)
            {
                mpData[i] = value;
            }
        }

        mnSize = size;
    }

    void insert(size_t index, const TYPE& data)
    {
        Assert(index <= mnSize);

        resize(mnSize + 1);
        TYPE* p = mpData + index;
        memmove(p + 1, p, (mnSize - index - 1) * sizeof(TYPE));
        *p = data;
    }

    void remove(size_t index)
    {
        Assert(index <= mnSize);

        TYPE* p = mpData + index;
        memmove(p, p + 1, (mnSize - index - 1) * sizeof(TYPE));
        --mnSize;
    }

    //��start��ʼ�Ƴ�count��Ԫ��
    void remove_some(size_t start, size_t count)
    {
        Assert((start <= mnSize) && ((start + cout) <= mnSize));
        TYPE* p = mpData + start;
        memmove(p, p + count, (mnSize - index - count) * sizeof(TYPE));
        mnSize -= count;
    }

    void clear()
    {
        mnSize = 0;
    }

    size_t get_mem_usage() const
    {
        size_t size = sizeof(self_t);
        if (mnCapacity > size)
        {
            size += mnCapacity * sizeof(TYPE);
        }

        return size;
    }
private:
    ALLOC mxAlloc;
    TYPE mxStack[SIZE];
    TYPE* mpData;
    size_t mnCapacity; //����
    size_t mnSize; //����
};