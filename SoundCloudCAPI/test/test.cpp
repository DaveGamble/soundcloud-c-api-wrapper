#include "../SoundCloudCAPI.h"
#include <stdio.h>

int main(int argc, char* argv[])
{

	SoundCloudCAPI *scAPI=SoundCloudCAPI_CreateWithDefaultCallbackAndGetCredentials("UF6IeQtnCyWHu4gxKbGeDQ","7t94mY84FtvXSqhveryhYRzCGGJoKHlLi6BannW9Cg","",1);

	while (SoundCloudCAPI_EvaluateCredentials(scAPI))
	{
		printf("Please Auth, then press enter!\n");
		getchar();
	}

	void *data;unsigned long long datalen;int errnum;
	int res=SoundCloudCAPI_performMethod(scAPI,"GET","/users/12708/tracks",0,0,&errnum,&data,&datalen);
	if (!res) printf("Got back: %s\n",data);

	SoundCloudCAPI_Delete(scAPI);

	getchar();

	return 0;
}

