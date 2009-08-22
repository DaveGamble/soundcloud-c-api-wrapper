#include "SoundCloudCAPI.h"
#include "SoundCloudCAPI_Internal.h"
#include <Carbon/Carbon.h>


void SoundCloudCAPI_DefaultAuthenticationOpenAuthURL(SoundCloudCAPI *api)
{
	const char *url;CFURLRef cfurl;
	url=SoundCloudCAPI_GetUserAuthorizationURL(api);
	cfurl=CFURLCreateWithBytes(0,(const UInt8*)url,strlen(url),0,0);
	LSOpenCFURLRef(cfurl,0);
	CFRelease(cfurl);
}

void SoundCloudCAPI_DefaultAuthenticationSave(SoundCloudCAPI *api)
{
	SecKeychainItemRef ref;
	const char *data=SoundCloudCAPI_GetCredentials(api);
	const char *service=api->apiBaseURL;
	CFStringRef bundleName=CFBundleGetIdentifier(CFBundleGetMainBundle());
	const char *name=CFStringGetCStringPtr(bundleName,kCFStringEncodingMacRoman);
	
	
	OSErr status=SecKeychainFindGenericPassword(NULL,strlen(name),name,strlen(service),service,0,0,&ref);
	if (status) status=SecKeychainAddGenericPassword(NULL,strlen(name),name,strlen(service),service,strlen(data),data,0);
	else		{status=SecKeychainItemModifyAttributesAndData(ref,NULL,strlen(data),data);CFRelease(ref);}
	if (status)	sc_log(api,SoundCloudCAPI_LogLevel_Errors,"Failed to save token, with err: %d\n",status);
	
	CFRelease(bundleName);
}


void SoundCloudCAPI_DefaultAuthenticationLoad(SoundCloudCAPI *c)
{
	const char *service,*name;OSStatus status;CFStringRef bundleName;
	void*data;UInt32 datalen;
	
	service=c->apiBaseURL;
	bundleName=CFBundleGetIdentifier(CFBundleGetMainBundle());
	name=CFStringGetCStringPtr(bundleName,kCFStringEncodingMacRoman);
	
	status=SecKeychainFindGenericPassword(NULL,strlen(name),name,strlen(service),service,&datalen,&data,NULL);
	if (!status)
	{
		char *buffer=(char*)malloc(datalen+1);memcpy(buffer,data,datalen);buffer[datalen]=0;
	
		SoundCloudCAPI_SetCredentials(c,buffer);
	
		free(buffer);
		SecKeychainItemFreeContent(NULL,data);
	}
	CFRelease(bundleName);
}



