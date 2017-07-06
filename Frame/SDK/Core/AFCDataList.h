#pragma once

#include "AFIData.h"
#include "AFIDataList.h"

namespace ArkFrame
{

class AFDataListAlloc
{
public:
    AFDataListAlloc() {}
    ~AFDataListAlloc() {}

    void* Alloc(size_t size)
    {
        return new char[size];
    }

    void Free(void* ptr, size_t size)
    {
        delete[] (char*)ptr;
    }

    void Swap(AFDataListAlloc& src)
    {
    }
};

template<size_t DATA_SIZE, size_t BUFFER_SIZE, typename ALLOC = AFDataListAlloc>
class AFCDataList : public AFIDataList
{
private:
    using self_t = AFCDataList<DATA_SIZE, BUFFER_SIZE, ALLOC>;

    struct dynamic_data_t
    {
        int nType;
        union
        {
            bool mbValue;
            int mnValue;
            int64_t mn64Value;
            float mfValue;
            double mdValue;
            size_t mnstrValue;
            void* mpVaule;
            size_t mnUserData;

            struct
            {
                uint32_t mnIdent;
                uint32_t mnSerial;
            };
        };
    };

public:
    AFCDataList()
    {
        assert(DATA_SIZE > 0);
        assert(BUFFER_SIZE > 0);

        mpData = mDataStack;
        mnDataSize = DATA_SIZE;
        mnDataUsed = 0;

        mpBuffer = mBufferStack;
        mnBufferSize = BUFFER_SIZE;
        mnBufferUsed = 0;
    }

    AFCDataList(const self_t& src)
    {
        assert(DATA_SIZE > 0);
        assert(BUFFER_SIZE > 0);

        mpData = mDataStack;
        mnDataSize = DATA_SIZE;
        mnDataUsed = 0;

        mpBuffer = mBufferStack;
        mnBufferSize = BUFFER_SIZE;
        mnBufferUsed = 0;
        InnerAppend(src, 0, src.GetCount());
    }

    virtual ~AFCDataList()
    {
        Release();
    }

    self_t operator==(const self_t src)
    {
        Release();

        mpData = mDataStack;
        mnDataSize = DATA_SIZE;
        mnDataUsed = 0;

        mpBuffer = mBufferStack;
        mnBufferSize = BUFFER_SIZE;
        mnBufferUsed = 0;
        InnerAppend(src, 0, src.GetCount());
        
        return *this;
    }

    void Release()
    {
        if (mnDataSize > DATA_SIZE)
        {
            mxAlloc.Free(mpData, mnDataSize * sizeof(dynamic_data_t));
        }

        if (mnBufferSize > BUFFER_SIZE)
        {
            mxAlloc.Free(mpBuffer, mnBufferSize * sizeof(char));
        }
    }

    virtual bool Concat(const AFIDataList& src)
    {
        InnerAppend(src, 0, src.GetCount());
        return true;
    }

    virtual bool Append(const AFIData& data)
    {
        InnerAppend(data);
        return true;
    }

    virtual bool Append(const AFIDataList& src, size_t start, size_t count)
    {
        if (start >= src.GetCount())
        {
            return false;
        }

        size_t end = start + count;
        if (end > src.GetCount())
        {
            return false;
        }

        InnerAppend(src, start, end);
        return true;
    }

    virtual void Clear()
    {
        mnDataUsed = 0;
        mnBufferUsed = 0;
    }

    virtual bool Empty() const
    {
        return (0 == mnDataUsed);
    }

    virtual size_t GetCount() const
    {
        return mnDataUsed;
    }

    virtual int GetType(size_t index) const
    {
        if (index >= mnDataUsed)
        {
            return DT_UNKNOWN;
        }

        return mpData[index].nType;
    }

    //add data
    virtual bool AddBool(bool value)
    {
        dynamic_data_t* p = AddDynamicData();
        p->nType = DT_BOOLEAN;
        p->mbValue = value;
        return true;
    }

    virtual bool AddInt(int value)
    {
        dynamic_data_t* p = AddDynamicData();
        p->nType = DT_INT;
        p->mnValue = value;
        return true;
    }

    virtual bool AddInt64(int64_t value)
    {
        dynamic_data_t* p = AddDynamicData();
        p->nType = DT_INT64;
        p->mn64Value = value;
        return true;
    }

    virtual bool AddFloat(float value)
    {
        dynamic_data_t* p = AddDynamicData();
        p->nType = DT_FLOAT;
        p->mfValue = value;
        return true;
    }

    virtual bool AddDouble(double value)
    {
        dynamic_data_t* p = AddDynamicData();
        p->nType = DT_DOUBLE;
        p->mdValue = value;
        return true;
    }

    virtual bool AddString(const char* value)
    {
        assert(NULL != value);
        dynamic_data_t* p = AddDynamicData();
        p->nType = DT_STRING;
        p->mnstrValue = mnBufferUsed;

        const size_t value_size = strlen(value) + 1;
        char* data = AddBuffer(value_size);
        memcpy(data, value, value_size);

        return true;
    }

    virtual bool AddObject(const AFGUID& value)
    {
        dynamic_data_t* p = AddDynamicData();
        p->nType = DT_OBJECT;
        p->mnIdent = value.nIdent;
        p->mnSerial = value.nSerial;
        return true;
    }

    virtual bool AddPointer(void* value)
    {
        dynamic_data_t* p = AddDynamicData();
        p->nType = DT_POINTER;
        p->mpVaule = value;
        return true;
    }

    virtual bool AddUserData(const void* pData, size_t size)
    {
        assert(NULL != pData);

        dynamic_data_t* p = AddDynamicData();
        p->nType = DT_USERDATA;
        p->mnUserData = mnBufferUsed;

        const size_t value_size = AFIData::GetRawUserDataSize(size);
        char* value = AddBuffer(value_size);
        AFIData::InitRawUserData(value, pData, size);

        return true;
    }

    virtual bool AddRawUserData(void* value)
    {
        return AddUserData(AFIData::GetUserData(value), AFIData::GetUserDataSize(value));
    }

    //get data
    virtual bool Bool(size_t index) const
    {
        if (index > mnDataUsed)
        {
            return NULL_BOOLEAN;
        }

        switch (mpData[index].nType)
        {
        case DT_BOOLEAN:
            return mpData[index].mbValue;
            break;
        default:
            break;
        }

        return NULL_BOOLEAN;
    }

    virtual int Int(size_t index) const
    {
        if (index > mnDataUsed)
        {
            return NULL_INT;
        }

        switch (mpData[index].nType)
        {
        case DT_INT:
            return mpData[index].mnValue;
            break;
        default:
            break;
        }

        return NULL_INT;
    }

    virtual int64_t Int64(size_t index) const
    {
        if (index > mnDataUsed)
        {
            return NULL_INT64;
        }

        switch (mpData[index].nType)
        {
        case DT_INT64:
            return mpData[index].mn64Value;
            break;
        default:
            break;
        }

        return NULL_INT64;
    }

    virtual float Float(size_t index) const
    {
        if (index > mnDataUsed)
        {
            return NULL_FLOAT;
        }

        switch (mpData[index].nType)
        {
        case DT_FLOAT:
            return mpData[index].mfValue;
            break;
        default:
            break;
        }

        return NULL_FLOAT;
    }

    virtual double Double(size_t index) const
    {
        if (index > mnDataUsed)
        {
            return NULL_DOUBLE;
        }

        switch (mpData[index].nType)
        {
        case DT_DOUBLE:
            return mpData[index].mdValue;
            break;
        default:
            break;
        }

        return NULL_DOUBLE;
    }

    virtual const char* String(size_t index) const
    {
        if (index > mnDataUsed)
        {
            return NULL_STR.c_str();
        }

        switch (mpData[index].nType)
        {
        case DT_STRING:
            return mpBuffer + mpData[index].mnstrValue;
            break;
        default:
            break;
        }

        return NULL_STR.c_str();
    }

    virtual AFGUID Object(size_t index) const
    {
        if (index > mnDataUsed)
        {
            return NULL_GUID;
        }

        switch (mpData[index].nType)
        {
        case DT_OBJECT:
            return AFGUID(mpData[index].mnIdent, mpData[index].mnSerial);
            break;
        default:
            break;
        }

        return NULL_GUID;
    }

    virtual void* Pointer(size_t index) const
    {
        if (index > mnDataUsed)
        {
            return NULL;
        }

        switch (mpData[index].nType)
        {
        case DT_STRING:
            return mpData[index].mpVaule;
            break;
        default:
            break;
        }

        return NULL;
    }

    virtual const void* UserData(size_t index, size_t& size) const
    {
        if (index > mnDataUsed)
        {
            size = 0;
            return NULL;
        }

        switch (mpData[index].nType)
        {
        case DT_USERDATA:
            {
                char* p = mpBuffer + mpData[index].mnUserData;
                size = AFIData::GetUserDataSize(p);
                return AFIData::GetUserData(p);
            }
            break;
        default:
            break;
        }

        size = 0;
        return NULL;
    }

    virtual void* RawUserData(size_t index) const
    {
        if (index > mnDataUsed)
        {
            return NULL;
        }

        switch (mpData[index].nType)
        {
        case DT_USERDATA:
            {
                return mpBuffer + mpData[index].mnUserData;
            }
            break;
        default:
            break;
        }

        return NULL;
    }

    virtual bool SetBool(size_t index, bool value)
    {
        if (index >= mnDataUsed)
        {
            return false;
        }

        if (mpData[index].nType != DT_BOOLEAN)
        {
            return false;
        }

        mpData[index].mbValue = value;
        return true;
    }

    virtual bool SetInt(size_t index, int value)
    {
        if (index >= mnDataUsed)
        {
            return false;
        }

        if (mpData[index].nType != DT_INT)
        {
            return false;
        }

        mpData[index].mnValue = value;
        return true;
    }

    virtual bool SetInt64(size_t index, int value)
    {
        if (index >= mnDataUsed)
        {
            return false;
        }

        if (mpData[index].nType != DT_INT64)
        {
            return false;
        }

        mpData[index].mn64Value = value;
        return true;
    }

    virtual bool SetFloat(size_t index, float value)
    {
        if (index >= mnDataUsed)
        {
            return false;
        }

        if (mpData[index].nType != DT_FLOAT)
        {
            return false;
        }

        mpData[index].mfValue = value;
        return true;
    }

    virtual bool SetDouble(size_t index, double value)
    {
        if (index >= mnDataUsed)
        {
            return false;
        }

        if (mpData[index].nType != DT_DOUBLE)
        {
            return false;
        }

        mpData[index].mdValue = value;
        return true;
    }

    virtual bool SetString(size_t index, const char* value)
    {
        assert(NULL != value);

        if (index >= mnDataUsed)
        {
            return false;
        }

        if (mpData[index].nType != DT_STRING)
        {
            return false;
        }

        char* p = mpBuffer + mpData[index].mnstrValue;
        const size_t size1 = strlen(value) + 1;

        if (size1 <= (strlen(p) + 1))
        {
            //���value�ĳ��� <= ��ǰ�ĳ��ȣ��Ż�ԭ�أ������ı�
            strcpy(p, value);
            return true;
        }

        mpData[index].mnstrValue = mnDataUsed;
        const size_t value_size = strlen(value) + 1;
        char* v = AddBuffer(value_size);
        memcpy(v, value, value_size);

        return true;
    }

    virtual bool SetObject(size_t index, const AFGUID& value)
    {
        if (index >= mnDataUsed)
        {
            return false;
        }

        if (mpData[index].nType != DT_OBJECT)
        {
            return false;
        }

        mpData[index].mnIdent = value.nIdent;
        mpData[index].mnSerial = value.nSerial;
        return true;
    }

    virtual bool SetPointer(size_t index, void* value)
    {
        if (index >= mnDataUsed)
        {
            return false;
        }

        if (mpData[index].nType != DT_POINTER)
        {
            return false;
        }

        mpData[index].mpVaule = value;
        return true;
    }

    virtual size_t GetMemUsage() const
    {
        size_t size = sizeof(self_t);
        if (mnDataSize > DATA_SIZE)
        {
            size += sizeof(dynamic_data_t) * mnDataSize;
        }

        if (mnBufferSize > BUFFER_SIZE)
        {
            size += sizeof(char) * mnBufferSize;
        }

        return size;
    }

protected:
    dynamic_data_t* AddDynamicData()
    {
        if (mnDataUsed >= mnDataSize)
        {
            size_t new_size = mnDataSize * 2;
            dynamic_data_t* p = (dynamic_data_t*)mxAlloc.Alloc(new_size * sizeof(dynamic_data_t));
            memcpy(p, mpData, mnDataUsed * sizeof(dynamic_data_t));
            if (mnDataSize > DATA_SIZE)
            {
                mxAlloc.Free(mpData, mnDataSize * sizeof(dynamic_data_t));
            }

            mpData = p;
            mnDataSize = new_size;
        }

        return mpData + mnDataUsed++;
    }

    char* AddBuffer(size_t need_size)
    {
        size_t new_used = mnBufferUsed + need_size;
        if (new_used > mnBufferSize)
        {
            size_t new_size = mnBufferSize * 2;
            if (new_used > new_size)
            {
                new_size = new_used * 2;
            }

            char* p = (char*)mxAlloc.Alloc(new_size);
            memcpy(p, mpBuffer, mnBufferUsed);

            if (mnBufferSize > BUFFER_SIZE)
            {
                mxAlloc.Free(mpBuffer, mnBufferSize);
            }

            mpBuffer = p;
            mnBufferSize = new_size;
        }

        char* ret = mpBuffer + mnBufferUsed; //���ص��Ǽ�buffer֮ǰ��λ��
        mnBufferUsed = new_used;
        return ret;
    }

    void InnerAppend(const AFIData& data)
    {
        switch (data.GetType())
        {
        case DT_BOOLEAN:
            AddBool(data.GetBool());
            break;
        case DT_INT:
            AddInt(data.GetInt());
            break;
        case DT_INT64:
            AddInt64(data.GetInt64());
            break;
        case DT_FLOAT:
            AddFloat(data.GetFloat());
            break;
        case DT_DOUBLE:
            AddDouble(data.GetDouble());
            break;
        case DT_STRING:
            AddString(data.GetString());
            break;
        case DT_OBJECT:
            AddObject(data.GetObject());
            break;
        case DT_POINTER:
            AddPointer(data.GetPointer());
            break;
        case DT_USERDATA:
            {
                size_t size;
                const void* pData = data.GetUserData(size);
                AddUserData(pData, size);
            }
            break;
        default:
            assert(0);
            break;
        }
    }

    void InnerAppend(const AFIDataList& src, size_t start, size_t end)
    {
        for (size_t i = start; i < end; ++i)
        {
            switch (src.GetType(i))
            {
            case DT_BOOLEAN:
                AddBool(src.Bool(i));
                break;
            case DT_INT:
                AddInt(src.Int(i));
                break;
            case DT_INT64:
                AddInt64(src.Int64(i));
                break;
            case DT_FLOAT:
                AddFloat(src.Float(i));
                break;
            case DT_DOUBLE:
                AddDouble(src.Double(i));
                break;
            case DT_STRING:
                AddString(src.String(i));
                break;
            case DT_OBJECT:
                AddObject(src.Object(i));
                break;
            case DT_POINTER:
                AddPointer(src.Pointer(i));
                break;
            case DT_USERDATA:
                {
                    size_t size;
                    const void* pData = src.UserData(i, size);
                    AddUserData(pData, size);
                }
                break;
            default:
                assert(0);
                break;
            }
        }
    }

private:
    ALLOC mxAlloc;
    dynamic_data_t mDataStack[DATA_SIZE];
    dynamic_data_t* mpData;
    size_t mnDataSize;
    size_t mnDataUsed;
    char mBufferStack[BUFFER_SIZE];
    char* mpBuffer;
    size_t mnBufferSize;
    size_t mnBufferUsed;
};

using AFXDataList = AFCDataList<8, 128>;

}