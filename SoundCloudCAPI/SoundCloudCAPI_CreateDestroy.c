#include "SoundCloudCAPI_Internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

extern void SoundCloudCAPI_CreateFor(SoundCloudCAPI *c,const char *consumerKey,const char *consumerSecret,const char* callbackURL,int productionType);

SoundCloudCAPI *SoundCloudCAPI_Create(const char *consumerKey,const char *consumerSecret,const char* callbackURL,int productionType)
{
	SoundCloudCAPI *c=(SoundCloudCAPI*)malloc(sizeof(SoundCloudCAPI));
	
	c->authDelegate=0;
	c->authDelegateData=0;
	
	c->responseFormat=SCResponseFormatJSON;	// I think this is the new default.
	
	c->credentials=0;
	c->auth_status=0;	// as yet, no credentials. don't try and do anything yet!!
	
	c->t_key=0;
	c->t_secret=0;
	c->t_type=0;
	
	c->userAuthURL=0;
	
	SoundCloudCAPI_CreateFor(c,consumerKey,consumerSecret,callbackURL,productionType);
	
	c->log_level=SoundCloudCAPI_LogLevel_Silent;
	
	return c;
}

SoundCloudCAPI *SoundCloudCAPI_CreateWithCallback(const char *consumerKey,const char *consumerSecret,const char* callbackURL,int productionType,SoundCloudCAuthenticationCallback* authDelegate,void *user)
{
	SoundCloudCAPI *c=SoundCloudCAPI_Create(consumerKey,consumerSecret,callbackURL,productionType);
		
	c->authDelegate=authDelegate;
	c->authDelegateData=user;
	
	return c;
}

SoundCloudCAPI *SoundCloudCAPI_CreateWithDefaultCallbackAndGetCredentials(const char *consumerKey,const char *consumerSecret,const char* callbackURL,int productionType)
{
	SoundCloudCAPI *c=SoundCloudCAPI_CreateWithCallback(consumerKey,consumerSecret,callbackURL,productionType,SoundCloudCAPI_DefaultAuthenticationCallback,NULL);	
	SoundCloudCAPI_DefaultAuthenticationLoad(c);
	return c;
}

void SoundCloudCAPI_Delete(SoundCloudCAPI *api)
{
	if (api->credentials)	free(api->credentials);
	if (api->t_key)			free(api->t_key);			// TODO: Should we scrub these more aggressively??
	if (api->t_secret)		free(api->t_secret);
	if (api->userAuthURL)	free(api->userAuthURL);
	if (api->consumerKey)	free(api->consumerKey);
	if (api->consumerSecret)free(api->consumerSecret);
	if (api->callbackURL)	free(api->callbackURL);
	free(api);
}

void SoundCloudCAPI_SetResponseFormat(SoundCloudCAPI *api,int format){api->responseFormat=format;}
void SoundCloudCAPI_SetLogLevel(SoundCloudCAPI *api,int level){api->log_level=level;}

char *sc_strdup(const char *src){char *c=(char*)malloc(strlen(src)+1);strcpy(c,src);return c;}

void sc_log(SoundCloudCAPI *api,int level,const char *format,...)
{
	va_list vargs;
	va_start(vargs,format);
	if (level<=api->log_level)
	{
		vfprintf(stderr,format,vargs);
	}
	va_end(vargs);
}
