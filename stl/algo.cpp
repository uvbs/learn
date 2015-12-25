/*************************************************************************
    > File Name: algo.cpp
    > Author: sudoku.huang
    > Mail: sudoku.huang@gmail.com 
    > Created Time: Thu 19 Nov 2015 04:03:49 PM CST
 ************************************************************************/

#include <iostream>
#include <algorithm>
#include <vector>

int main()
{
    std::vector<int> v;
    for (int i = 0; i < 10; i ++)
        v.push_back(i);

    bool t = std::binary_search(v.begin(), v.end(), 4);
    std::cout << t << std::endl;

    typedef std::vector<int>::iterator vit;

    std::pair<vit, vit> it = std::equal_range(v.begin(), v.end(), 4);
    std::cout << * it.first << std::endl;
    std::cout << * it.second << std::endl;
    for (; it.first != it.second; ++ it.first)
    std::cout << * it.first << std::endl;

    return 1;
}
