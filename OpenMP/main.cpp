#include <iostream>
#include <string>
#include <fstream>
#include <omp.h>

bool isPalindrome(const std::string &s, int left, int right)
{
    while (left < right)
    {
        if (s[left++] != s[right--])
        {
            return false;
        }
    }
    return true;
}

void findPalindromes(const std::string &s)
{
    int len = s.size();

#pragma omp parallel for
    for (int i = 0; i < len; ++i)
    {
        for (int j = i; j < len; ++j)
        {
            if (isPalindrome(s, i, j))
            {
#pragma omp critical
                {
                    std::cout << i << " " << j << std::endl;
                }
            }
        }
    }

#pragma omp parallel
    {
#pragma omp single
        {
            std::cout << "Number of threads = " << omp_get_num_threads() << std::endl;
        }
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename>" << std::endl;
        return 1;
    }

    std::ifstream inFile(argv[1]);
    if (!inFile)
    {
        std::cerr << "Could not open file " << argv[1] << std::endl;
        return 1;
    }

    std::string text((std::istreambuf_iterator<char>(inFile)), std::istreambuf_iterator<char>());

    findPalindromes(text);
    return 0;
}
