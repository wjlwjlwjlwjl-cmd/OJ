# problem 1
#include <iostream>
#include <vector>
#include <unordered_map>
#include <cassert> 
using namespace std; 
class Solution 
{
public:     
vector<int> twoSum(vector<int>& nums, int target) 
{ 
    unordered_map<int, int> hash; 
    for(int i = 0; i < nums.size(); i++)
    {
        int x = target - nums[i];
        if(hash.count(x) != 0)
        { 
            return {hash[x], i};}
            hash[nums[i]] = i;
        }
        return {-1, -1};
    }
};
