#include "../SoundCloudCAPI.h"
#include <stdio.h>

int main(int argc, char* argv[])
{

	SoundCloudCAPI *scAPI=SoundCloudCAPI_CreateWithDefaultCallbackAndGetCredentials("UF6IeQtnCyWHu4gxKbGeDQ","7t94mY84FtvXSqhveryhYRzCGGJoKHlLi6BannW9Cg","",1);
	SoundCloudCAPI_SetLogLevel(scAPI, SoundCloudCAPI_LogLevel_Debug);

	char buffer[256];int code;
	while (SoundCloudCAPI_EvaluateCredentials(scAPI))
	{
		printf("Please Auth, then enter the verification code here: ");
		scanf("%d",&code);
		sprintf(buffer,"%d",code);
		SoundCloudCAPI_SetVerifier(scAPI, buffer);
	}

	void *data;unsigned long long datalen;int errnum;
	int res=SoundCloudCAPI_performMethod(scAPI,"GET","/users/12708/tracks",0,0,&errnum,&data,&datalen);
	if (!res) printf("Got back: %s\n",data);

	SoundCloudCAPI_Delete(scAPI);

	getchar();

	return 0;
}

