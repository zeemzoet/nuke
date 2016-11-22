// Example program
#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

using namespace std;

int main()
{
    vector<float> v;
    float test = 1.0f;
    
    for (int i = 0; i<5; i++)
        v.push_back(i*0.1f);
    
    size_t n = v.size() * test;
    //fix so with a 'test' value of 1, it still chooses the last element of the vector, not the first
    if (n == v.size())
        n--;
    
    
    nth_element(v.begin(), v.begin()+n, v.end());
    
    cout << n <<"\n";
    cout << v[n];
}
