// Manage the APIConfiguration objects.
// TODO: Check parameters are valid... FAIL if we get nil values.

#include "SoundCloudCAPI_Internal.h"

#include <string.h>
#include <stdlib.h>

// URLs:
// Production:
#define kSoundCloudAPIURL				"http://api.soundcloud.com"
#define kSoundCloudAPIRequestTokenURL	"http://api.soundcloud.com/oauth/request_token"
#define kSoundCloudAPIAccesTokenURL		"http://api.soundcloud.com/oauth/access_token"
#define kSoundCloudAuthURL				"http://soundcloud.com/oauth/authorize"
// Sandbox:
#define kSoundCloudSandboxAPIURL				"http://api.sandbox-soundcloud.com"
#define kSoundCloudSandboxAPIRequestTokenURL	"http://api.sandbox-soundcloud.com/oauth/request_token"
#define kSoundCloudSandboxAPIAccesTokenURL		"http://api.sandbox-soundcloud.com/oauth/access_token"
#define kSoundCloudSandboxAuthURL				"http://sandbox-soundcloud.com/oauth/authorize"

static void SetKeys(SoundCloudCAPI *config,const char *consumerKey,const char* consumerSecret,const char *callbackURL)
{
	config->consumerKey		=sc_strdup(consumerKey);
	config->consumerSecret	=sc_strdup(consumerSecret);
	if (callbackURL && strlen(callbackURL))	config->callbackURL		=sc_strdup(callbackURL);
	else									config->callbackURL		=sc_strdup("oob");
}

static void SoundCloudCAPI_CreateForProduction(SoundCloudCAPI *c,const char *consumerKey,const char *consumerSecret,const char* callbackURL)
{
	SetKeys(c,consumerKey,consumerSecret,callbackURL);
	c->apiBaseURL		=kSoundCloudAPIURL;
	c->requestTokenURL	=kSoundCloudAPIRequestTokenURL;
	c->accessTokenURL	=kSoundCloudAPIAccesTokenURL;
	c->authURL			=kSoundCloudAuthURL;
}

static void SoundCloudCAPI_CreateForSandbox(SoundCloudCAPI *c,const char *consumerKey,const char *consumerSecret,const char* callbackURL)
{
	SetKeys(c,consumerKey,consumerSecret,callbackURL);
	c->apiBaseURL		=kSoundCloudSandboxAPIURL;
	c->requestTokenURL	=kSoundCloudSandboxAPIRequestTokenURL;
	c->accessTokenURL	=kSoundCloudSandboxAPIAccesTokenURL;
	c->authURL			=kSoundCloudSandboxAuthURL;
}

void SoundCloudCAPI_CreateFor(SoundCloudCAPI *c,const char *consumerKey,const char *consumerSecret,const char* callbackURL,int productionType)
{
	if (productionType==SCProductionType_Production) SoundCloudCAPI_CreateForProduction(c, consumerKey, consumerSecret, callbackURL);
	else	SoundCloudCAPI_CreateForSandbox(c, consumerKey, consumerSecret, callbackURL);
}