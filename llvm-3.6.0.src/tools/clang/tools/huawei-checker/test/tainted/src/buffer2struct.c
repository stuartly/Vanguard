#include "protype.h"

struct Message
{
		unsigned int ID;
		unsigned char Name[5];
		unsigned char sex;
};

struct Message *one=NULL;
struct Message *two=NULL;

static unsigned char array[5]={'a','b','c','d','e'};

void main()
{
	struct Message *msg1=(struct Message *)array;
	one=msg1;
	one->ID=1;
	for(int i=0;i<sizeof(one->Name);i++)
	{
		one->Name[i]='a';
	}
	one->sex='M';
	
	int structSize=sizeof(struct Message);
	
	struct Message *msg2=(struct Message *)malloc(structSize/2);
	two=msg2;
	two->ID=2;
	
	for(int i=0;i<sizeof(two->Name);i++)
	{
		two->Name[i]='a';
	}
	msg1->sex='M';
	

	
}