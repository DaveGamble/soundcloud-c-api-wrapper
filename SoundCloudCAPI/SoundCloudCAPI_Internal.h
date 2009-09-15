#ifndef SoundCloudCAPIInternal__H
#define SoundCloudCAPIInternal__H

#include "SoundCloudCAPI.h"
#include <stdio.h>

extern char *sc_strdup(const char *src);
extern void sc_log(SoundCloudCAPI *api,int level,const char *format,...);

struct _SoundCloudCAPI
{	
	SoundCloudCAuthenticationCallback *authDelegate;
	void *authDelegateData;

	char *credentials;
	char *userAuthURL;
	char *verifier;
	
	char *t_key,*t_secret;
	int t_type;

	int auth_status;
	int responseFormat;
	
	const char *apiBaseURL;
	const char *requestTokenURL;
	const char *accessTokenURL;
	const char *authURL;

	char *consumerKey;
	char *consumerSecret;
	char *callbackURL;
	
	int log_level;
};

typedef struct
{
	SoundCloudCAPI *api;
	SoundCloudCAPICallback callback;
	void *callback_data;
	char *url;
	char *header;
	char *body;
	char *httpMethod;
	size_t bodylen;
	long error;
	
} SoundCloudCAPIThread;

#endif