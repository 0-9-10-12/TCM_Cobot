#include "RingBuffer.h"

RingBuffer::RingBuffer()
    : write_point(0)
    , read_point(0)
{
}

RingBuffer::~RingBuffer()
{
}

int RingBuffer::enqueue(char* data, int data_size)
{
    if (data_size > BUFFER_SIZE) // �������� ũ�⺸�� �� ū �����Ͱ� ����
    {
        return -1;
    }

    int w_p = write_point;
    for (int i{}; i < data_size; ++i)
    {
        if ((w_p == read_point) && !empty()) // ������ ���� �����Ͱ� �־ �� �� ��. ���� ǥ��
        {
            return -2; 
        }

        ++w_p;

        if (w_p == BUFFER_SIZE)
        {
            w_p = 0;
        }
    }

    memcpy(buffer + write_point, data, data_size);

    write_point = w_p;

    return data_size;

    //int write_num = 0;

    //// ���� ���۸� �Ѿ�ٸ� ��� ä��ٰ� ������ �����ͺ��� ����� �ϱ�
    //if (BUFFER_SIZE <= write_point + data_size)
    //{
    //    int remain1 = BUFFER_SIZE - write_point;
    //    memcpy(buffer + write_point, data, remain1);
    //    write_num += remain1;
    //    write_point = 0;

    //    int remain2 = data_size - remain1;
    //    memcpy(buffer + write_point, data + remain1, remain2);
    //    write_num += remain2;
    //    write_point += remain2;

    //    return write_num;
    //}

    //memcpy(buffer + write_point, data, data_size);

    //write_point += data_size;

    //return data_size;
}

int RingBuffer::dequeue(char* dest, int data_size)
{
    if (data_size > BUFFER_SIZE) // ���� ũ�⺸�� �� ū ũ�⸦ �������� �� ��
    {
        return -1;
    }

    int r_p = read_point;
    for (int i{}; i < data_size; ++i)
    {
        if (r_p == write_point) // �� ������ ���µ� �д� �ٴ� ���̹Ƿ� ����
        {
            return -2;
        }

        ++r_p;

        if (r_p == BUFFER_SIZE)
        {
            r_p = 0;
        }
    }

    memcpy(dest, buffer + read_point, data_size);
    read_point = r_p;

    return data_size;

    //int read_num = 0;

    //// n + 1 �� ���ٸ�?
    //if (BUFFER_SIZE <= read_point + data_size)
    //{
    //    int remain1 = BUFFER_SIZE - read_point;
    //    memcpy(dest, buffer + read_point, remain1);
    //    read_num += remain1;
    //    read_point = 0;

    //    int remain2 = data_size - remain1;
    //    memcpy(dest + remain1, buffer + read_point, remain2);
    //    read_num += remain2;
    //    read_point += remain2;

    //    return read_num;
    //}

    //memcpy(dest, buffer + read_point, data_size);
    //read_num += data_size;
    //read_point += data_size;

    //return read_num;
}

int RingBuffer::diff()
{
    int diff;

    if (read_point > write_point)
    {
        diff = (BUFFER_SIZE - read_point) + write_point;
    }
    else
    {
        diff = write_point - read_point;
    }
    return diff;
}

bool RingBuffer::empty()
{
    return (write_point == read_point);
}

void RingBuffer::move_read_pos(char move)
{
    if (BUFFER_SIZE <= read_point + move)
    {
        read_point = move - (BUFFER_SIZE - read_point);
    }
    else
    {
        read_point += move;
    }
}
