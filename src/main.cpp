#include <iostream>
#include <vector>

template <typename T>
void printVector(const std::vector<T>& vec)
{
    const std::size_t size{vec.size()};
    std::cout << "{";
    for (std::size_t i{0}; i < size; ++i)
    {
        std::cout << vec[i];
        if (i < size - 1)
        {
            std::cout << ", ";
        }
    }
    std::cout << "}\n";
}

std::vector<int> get_indices(std::vector<int> vertices)
{
    int T{static_cast<int>(vertices.size())};
    int H{static_cast<int>((T + 1) / 2)};
    int TQ{static_cast<int>((T - 1) / 2)};
    std::vector<int> indices;
    indices.resize(TQ * 6);

    for (int c{0}; c < TQ; ++c)
    {
        int qi {c * 6}; // quad index
        indices[qi] = c;
        indices[qi + 1] = c + 1;
        indices[qi + 2] = H + c;
        indices[qi + 3] = H + c;
        indices[qi + 4] = H + c + 1;
        indices[qi + 5] = c + 1;
    }

    return indices;
}

int main()
{
    std::vector<int> vertices {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    std::vector<int> indices{get_indices(vertices)};
    printVector(indices);
    return 0;
}
