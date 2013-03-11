#include <cstdio>
#include <cstring>
#include <algorithm>

int FindMaxSubstring(const char* str) {
    int length = strlen(str);
    int max_length = 0;
    int bucket[128] = { };
    int bucket_size = 0;
    int head = 0;

    for (int i = 0; i < length; ++i) {
        ++bucket[str[i]];
        if (bucket[str[i]] == 1) {  // new character
            ++bucket_size;
            while (bucket_size > 2) {
                --bucket[str[head]];
                if (bucket[str[head]] == 0) {
                    --bucket_size;
                }
                ++head;
            }
        }
        
        max_length = std::max(max_length, i - head + 1);
    }

    return max_length;
}

int main() {
  for(int i=0;i<10000000;i++){
    const char* test = "ababababababababababaaaddddffffffffbdddddddssaaac"; 
    printf("%d\n",FindMaxSubstring(test));
  }
  /*
    const char* test1 = "aaaaaaabbbbbbcccccccdaaaaaaaaaaccccccccabb"; 
    printf("%s => %d\n", test1, FindMaxSubstring(test1));

    const char* test2 = "aaaaaa"; 
    printf("%s => %d\n", test2, FindMaxSubstring(test2));

    const char* test3 = "aaddddffffffffbdddddddssaaaaaaaaaaaaaaaaaaaabbbbbbbbbbbbbbbibcc"; 
    printf("%s => %d\n", test3, FindMaxSubstring(test3));
    */

    return 0;
}
