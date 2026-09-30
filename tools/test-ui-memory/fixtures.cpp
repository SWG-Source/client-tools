#if !defined(_MSC_VER) || _MSC_VER != 1800
#error This native fixture requires the VS2013 v120 compiler.
#endif
#include "_precompile.h"
#include "UiMemoryBlockManager.h"
#include "UIButton.h"
#include "UIPage.h"
#include "UIText.h"
#include <cstdint>
#include <cstdio>
unsigned checks=0;
#define CHECK(c) do{if(!(c)){std::printf("FAIL line %d\n",__LINE__);return false;}++checks;}while(0)
size_t stride(size_t size,size_t alignment){return (size+sizeof(size_t)+alignment)/alignment*alignment;}
bool test(size_t size,size_t alignment){
 UiMemoryBlockManager pool("alignment fixture",65536);void * blocks[4];
 for(unsigned i=0;i<4;++i){blocks[i]=pool.allocMem(size);CHECK(blocks[i]!=0);CHECK(reinterpret_cast<uintptr_t>(blocks[i])%alignment==0);if(i)CHECK(reinterpret_cast<uintptr_t>(blocks[i])-reinterpret_cast<uintptr_t>(blocks[i-1])==stride(size,sizeof(size_t)));}
 for(unsigned i=0;i<4;++i)pool.freeMem(blocks[i]);
 void * reused[4];for(unsigned i=0;i<4;++i){void * p=pool.allocMem(size);reused[i]=p;bool found=false;for(unsigned j=0;j<4;++j)if(p==blocks[j]){found=true;blocks[j]=0;break;}CHECK(found);CHECK(reinterpret_cast<uintptr_t>(p)%alignment==0);}
 for(unsigned i=0;i<4;++i)pool.freeMem(reused[i]);
 return true;
}
int main(){
 for(size_t s=1;s<=1024;++s){size_t n=stride(s,sizeof(size_t));if(n%sizeof(size_t)||n<s+sizeof(size_t)||(sizeof(size_t)==4&&n!=stride(s,4))){std::puts("FAIL arithmetic");return 1;}checks+=3;}
 if(!test(sizeof(UIButton),__alignof(UIButton))||!test(sizeof(UIPage),__alignof(UIPage))||!test(sizeof(UIText),__alignof(UIText)))return 1;
 std::printf("PASS: %u UI alignment checks\n",checks);return 0;
}
