#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pcre.h"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xmlmemory.h>
static unsigned allocs,frees;
static void *pm(size_t n){++allocs;return malloc(n);}
static void pf(void *p){++frees;free(p);}
int main(void){
 int u=0,n=0,l=0;const char *err=0;int off=0,vec[9];pcre *r;
 pcre_config(PCRE_CONFIG_UTF8,&u);pcre_config(PCRE_CONFIG_NEWLINE,&n);pcre_config(PCRE_CONFIG_LINK_SIZE,&l);
 if(u!=1||n!=10||l!=2)return 8;
 printf("PCRE version=%s utf8=%d newline=%d link=%d\n",pcre_version(),u,n,l);
 pcre_malloc=pm;pcre_free=pf;r=pcre_compile("^(ab)+$",0,&err,&off,0);if(!r)return 1;
 if(pcre_exec(r,0,"abab",4,0,0,vec,9)!=2)return 2;pcre_free(r);if(!allocs||!frees)return 3;
 r=pcre_compile("^.$",PCRE_UTF8,&err,&off,0);if(!r)return 9;
 if(pcre_exec(r,0,"\xc3\xa9",2,0,0,vec,9)!=1)return 10;pcre_free(r);
 {xmlDocPtr doc;xmlNodePtr node;xmlChar *buffer=0;int len=0;
 if(xmlMemSetup(free,malloc,realloc,_strdup)!=0)return 4;
 doc=xmlParseMemory("<root a='b'>value</root>",24);if(!doc)return 5;node=xmlDocGetRootElement(doc);if(!node||xmlStrcmp(node->name,(const xmlChar*)"root"))return 6;
 xmlDocDumpFormatMemory(doc,&buffer,&len,0);if(!buffer||len<=0)return 7;xmlFree(buffer);xmlFreeDoc(doc);
 doc=xmlNewDoc((const xmlChar*)"1.0");node=xmlNewNode(0,(const xmlChar*)"root");xmlDocSetRootElement(doc,node);xmlNewProp(node,(const xmlChar*)"a",(const xmlChar*)"b");xmlAddChild(node,xmlNewText((const xmlChar*)"value"));xmlFreeDoc(doc);xmlCleanupParser();}
 puts("PASS: actual PCRE hooks/match and libxml creation/parse/dump/free");return 0;
}
