#include "SoundCloudCAPI_Internal.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "oauth.h"


void SoundCloudCAPI_DefaultAuthenticationCallback(SoundCloudCAPI *api,int status,void *user)
{
	if (status & SCAuthenticationStatus_CredentialsHaveChanged)
	{
		SoundCloudCAPI_DefaultAuthenticationSave(api);
	}
	
	switch (status&255)
	{
		case SCAuthenticationStatus_UserMustAuthorize:
			SoundCloudCAPI_DefaultAuthenticationOpenAuthURL(api);
			break;
			
		case SCAuthenticationStatus_ErrorCouldNotRequest:
			sc_log(api,SoundCloudCAPI_LogLevel_Errors,"Could not request authorization from server!\n");
			break;
			
		case SCAuthenticationStatus_ErrorCouldNotAccess:
			sc_log(api,SoundCloudCAPI_LogLevel_Errors,"Could not request access from server!\n");
			break;
	}
}

// Credentials format:
// [R|A][\t][key][\t][secret][\0]
// R = request token, A = access token.
// key = OAuth key, secret = OAuth secret.

void SoundCloudCAPI_SetCredentials(SoundCloudCAPI *api,const char* credentials)
{
	char *ptr,*type,*key,*secret;int typelen=0,keylen=0,secretlen=0;
	if (api->credentials)	{free(api->credentials);api->credentials=0;}
	if (api->t_key)			{free(api->t_key);api->t_key=0;}
	if (api->t_secret)		{free(api->t_secret);api->t_secret=0;}
	api->t_type=0;

	if (!credentials) return;

	api->credentials=sc_strdup(credentials);	// Load these new credentials.
	// Unpack credential string!
	// Step 1: parse and verify:
	ptr=api->credentials;
	type=ptr;
	while (*ptr!=' ' && *ptr!=0) typelen++,ptr++;
	ptr++;
	key=ptr;
	while (*ptr!=' ' && *ptr!=0) keylen++,ptr++;
	ptr++;
	secret=ptr;
	while (*ptr!=0)	secretlen++,ptr++;
	if (typelen==1 && keylen && secretlen)	// we have some data for each!
	{
		api->t_key=(char*)malloc(keylen+1);			api->t_key[keylen]=0;		memcpy(api->t_key,key,keylen);
		api->t_secret=(char*)malloc(secretlen+1);	api->t_secret[secretlen]=0;	memcpy(api->t_secret,secret,secretlen);
		api->t_type=(*type=='R')?1:(*type=='A')?2:0;
	}
}

const char *SoundCloudCAPI_GetCredentials(SoundCloudCAPI *api)
{
	size_t credlen;
	if (api->t_type<=0 || api->t_type>2)	return	0;	// no credentials.
	if (!api->t_key || !api->t_secret)		return	0;
	
	if (api->credentials) {free(api->credentials);}
	
	credlen=4+strlen(api->t_key)+strlen(api->t_secret);
	api->credentials=(char*)malloc(credlen);
	sprintf(api->credentials,"%c %s %s",(api->t_type==1)?'R':'A',api->t_key,api->t_secret);
	
	return api->credentials;	
}

void SoundCloudCAPI_RemoveCredentials(SoundCloudCAPI *api)	{SoundCloudCAPI_SetCredentials(api, 0);}

const char *SoundCloudCAPI_GetUserAuthorizationURL(SoundCloudCAPI *api)
{
	return api->userAuthURL;
}

void SoundCloudCAPI_SetVerifier(SoundCloudCAPI *api, const char *verifier)
{
	if (api->verifier) free(api->verifier);
	api->verifier=sc_strdup(verifier);
}

// Subroutines for the AUTH code:
#define EV_RETURN(x)     {if (api->authDelegate) {(*api->authDelegate)(api,(x),api->authDelegateData);return SCAuthenticationStatus_WillCallback;}else return (x);}
#define EV_RETURNBOTH(x) {if (api->authDelegate) {(*api->authDelegate)(api,(x),api->authDelegateData);}return (x&255);}

static int parse_reply(char *reply, char **token, char **secret,int request)	// Taken from oauthexample.c from liboauth. Very slick style.
{
	int rc,failed=1;char **rv = NULL;

	rc = oauth_split_url_parameters(reply, &rv);
	qsort(rv, rc, sizeof(char *), oauth_cmpstringp);
	if( rc==2 && !strncmp(rv[0],"oauth_token=",11) && !strncmp(rv[1],"oauth_token_secret=",18) && !request)
	{
		failed=0;
		if (token)  *token =sc_strdup(&(rv[0][12]));
		if (secret) *secret=sc_strdup(&(rv[1][19]));
	}
	if( rc==3 && !strncmp(rv[0],"oauth_callback_confirmed=true",29) && !strncmp(rv[1],"oauth_token=",11) && !strncmp(rv[2],"oauth_token_secret=",18) && request)
	{
		failed=0;
		if (token)  *token =sc_strdup(&(rv[1][12]));
		if (secret) *secret=sc_strdup(&(rv[2][19]));
	}
	if(rv) free(rv);
	if(reply) free(reply);
	return failed;
}

// Build the user authorization url!
static void BuildUserAuthURL(SoundCloudCAPI *api)
{
	char *temp;size_t urllen;
	// if we have a callback url, escape it:
	if (api->callbackURL) temp=oauth_url_escape(api->callbackURL); else temp=sc_strdup("oob");
	// how long is the new url?
	urllen=strlen(api->authURL)+14+strlen(api->t_key);
	if (temp) urllen+=16+strlen(temp);	// add on callback if needs be
	// Allocate new space
	if (api->userAuthURL) free(api->userAuthURL);
	api->userAuthURL=(char*)malloc(urllen+1);api->userAuthURL[urllen]=0;
	// construct the new url!
	if (temp)	{sprintf(api->userAuthURL,"%s?oauth_token=%s&oauth_callback=%s",api->authURL,api->t_key,temp);free(temp);}
	else		 sprintf(api->userAuthURL,"%s?oauth_token=%s",api->authURL,api->t_key);	
	sc_log(api,SoundCloudCAPI_LogLevel_Debug,"Auth URL: [%s]\n",api->userAuthURL);
}

// we got a reply from the server, so store the keys and update the type!
static void updatekeys(SoundCloudCAPI *api,int newtype,char *newkey,char *newsecret, int *ret)
{
	api->t_type=newtype;
	if (api->t_key) {free(api->t_key);}if (api->t_secret) {free(api->t_secret);}
	api->t_key=newkey;api->t_secret=newsecret;
	(*ret)|=SCAuthenticationStatus_CredentialsHaveChanged;
}

// Main Auth function:
// This is a 3-state state machine.
// State 0: Clean slate. Transition: Ask for a REQUEST token from the server and construct the user-authorization url
// State 1: User has authorized, so try to convert the REQUEST token to an ACCESS token
// State 2: We have an ACCESS token. We are authorized.
// Our default behaviour is to attempt to run through. In some cases, user intervention may not be required.
// If it IS required, you'll get a UserMustAuthorize, which invites you to provide the URL from GetUserAuthorizationURL
// to the user, for their approval.
// You can call EvaluateCredentials as often as you like, although your results will be either:
// Errors - could not connect to the server for reasons beyond your control
// Authenticated - we're done
// or, more often: UserMustAuthorize, which means you're still waiting for user approval.
// Whenever you get a CredentialsHaveChanged, it's a good idea to store them to secure-storage.
int  SoundCloudCAPI_EvaluateCredentials(SoundCloudCAPI *api)
{
	char *postargs,*url,*reply,*newkey,*newsecret,*requrl,*accurl;int ret=0;
	int tryAndSeeIfWeAreAlreadyAuthorizedOnThisServer=0;	// TODO: Decide whether this should be true or false.
	switch (api->t_type)	// what type do we have now.
	{
	case 0:	// no authentication at all. Send request to server.
		// Get a signed URL to request authorisation
		requrl=malloc(strlen(api->requestTokenURL)+strlen(api->callbackURL)+20);	// 1.0a, build callback url into request token
		sprintf(requrl,"%s?oauth_callback=%s",api->requestTokenURL,strlen(api->callbackURL)?api->callbackURL:"oob");
		url = oauth_sign_url2(requrl, &postargs, OA_HMAC, 0, api->consumerKey, api->consumerSecret, NULL, NULL);
		sc_log(api,SoundCloudCAPI_LogLevel_Debug,"Request: [%s?%s]\n",url,postargs);
		free(requrl);
		// Send to server: (TODO: Make this asynch!!)
		reply = oauth_http_post(url,postargs);
		// Process results: Free temps, parse the reply.
		if (url) free(url); if (postargs) free(postargs);	// clear temps.
		if (api->t_key) {free(api->t_key);api->t_key=0;}	// these MUST be NULL!
		if (api->t_secret) {free(api->t_secret);api->t_secret=0;}
		if (!reply || parse_reply(reply,&newkey,&newsecret,1)) {EV_RETURN(SCAuthenticationStatus_ErrorCouldNotRequest);}
		// Wooh! New credentials! Form the user auth URL, and return the good news!
		updatekeys(api,1,newkey,newsecret,&ret);	// We have a request token!!
			
		// we COULD consider testing whether this is already authorised here... TODO: consider that.
		BuildUserAuthURL(api);

		// win!
		if (!tryAndSeeIfWeAreAlreadyAuthorizedOnThisServer)	EV_RETURN(SCAuthenticationStatus_UserMustAuthorize|ret);
	case 1:	// we have a request token. Can we turn it into an access token?
		// Sign the access url with our request token
		accurl=malloc(strlen(api->accessTokenURL)+(api->verifier?strlen(api->verifier):0)+20);
		sprintf(accurl,"%s?oauth_verifier=%s",api->accessTokenURL,api->verifier?api->verifier:"");
		url = oauth_sign_url2(accurl, &postargs, OA_HMAC, 0, api->consumerKey, api->consumerSecret, api->t_key, api->t_secret);
		sc_log(api,SoundCloudCAPI_LogLevel_Debug,"Access: [%s?%s]\n",url,postargs);
		free(accurl);
		// Off we go... (TODO: Make this asynch!!)
		reply = oauth_http_post(url,postargs);
		// Process results...
		if (url) free(url); if (postargs) free(postargs);
		// Did it work? (parse to new pointers to preserve old keys...)
		if (!reply)	EV_RETURN(SCAuthenticationStatus_ErrorCouldNotAccess|ret);	// If no reply, server is dead.
		if (parse_reply(reply,&newkey,&newsecret,0))	{BuildUserAuthURL(api);EV_RETURN(SCAuthenticationStatus_UserMustAuthorize|ret);} // If wrong reply, user prob hasn't authed.

		updatekeys(api,2,newkey,newsecret,&ret);	// we got the access token!!
			
		// Win!!
	case 2:	// we have an auth token!
		EV_RETURNBOTH(SCAuthenticationStatus_Authenticated|ret);
		break;
	}
	return SCAuthenticationStatus_ErrorCouldNotRequest;
}


