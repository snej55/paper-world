#include <iostream>
#include <array>

template <typename T>
void swap(T** val1, T** val2)
{
    T* temp {*val1}; // create temp for value of val1
    *val1 = *val2; // swap addresses
    *val2 = temp;
}

int main()
{
    int orig[8] {0, 0, 1, 1, 0, 0, 0, 0};
    int length{8};
    int** arr;
    arr = new int*[8];
    for (std::size_t i{0}; i < length; ++i)
    {
        arr[i] = new int;
        *(arr[i]) = orig[i];
    }

    int num{length};
    for (std::size_t i{0}; i < num; ++i)
    {
        if (i < length)
        {
            if (*(arr[i]) == 0)
            {
                swap(&(arr[i]), &(arr[length - 1]));
                --length;
            }
        }
    }

    num = length;

    for (std::size_t i{0}; i < num; ++i)
    {
        if (i < length)
        {
            if (*(arr[i]) == 0)
            {
                swap(&(arr[i]), &(arr[length - 1]));
                --length;
            }
        }
    }

    num = length;

    for (std::size_t i{0}; i < num; ++i)
    {
        if (i < length)
        {
            if (*(arr[i]) == 0)
            {
                swap(&(arr[i]), &(arr[length - 1]));
                --length;
            }
        }
    }

    for (std::size_t i{0}; i < 8; ++i)
    {
        std::cout << *(arr[i]) << '\n';
        delete arr[i];
    }

    delete arr;

    return 0;
}