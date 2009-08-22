// SoundCloudCAPI.h
// Dave Gamble, for SoundCloud 2009
// This is the main include for using the API.
#ifndef SoundCloudCAPI__H
#define SoundCloudCAPI__H

#ifdef SCAPI_WIN_DLL
#if defined(BUILDING_SCAPI)
#define API_EXTERN  __declspec(dllexport)
#else
#define API_EXTERN  __declspec(dllimport)
#endif
#else
#define API_EXTERN
#endif

#ifdef  __cplusplus
extern "C" {
#endif
////////////////////// API Itself
// The API object:
typedef struct _SoundCloudCAPI SoundCloudCAPI;
// Authentication callback:
typedef void (SoundCloudCAuthenticationCallback)(SoundCloudCAPI *api,int status,void *user);
// Production types:
#define SCProductionType_Sandbox 0
#define SCProductionType_Production 1
// Construction:
API_EXTERN SoundCloudCAPI *SoundCloudCAPI_Create(const char *consumerKey,const char *consumerSecret,const char* callbackURL,int productionType);
API_EXTERN SoundCloudCAPI *SoundCloudCAPI_CreateWithCallback(const char *consumerKey,const char *consumerSecret,const char* callbackURL,int productionType,SoundCloudCAuthenticationCallback* authDelegate,void *user);
API_EXTERN SoundCloudCAPI *SoundCloudCAPI_CreateWithDefaultCallbackAndGetCredentials(const char *consumerKey,const char *consumerSecret,const char* callbackURL,int productionType);
API_EXTERN void SoundCloudCAPI_Delete(SoundCloudCAPI *api);
// Authentication functions:
API_EXTERN void SoundCloudCAPI_SetCredentials(SoundCloudCAPI *api,const char* credentials);	// Load credentials from secure location on disk
API_EXTERN const char *SoundCloudCAPI_GetCredentials(SoundCloudCAPI *api);						// Get current credentials to store to disk
API_EXTERN void SoundCloudCAPI_RemoveCredentials(SoundCloudCAPI *api);							// calls SoundCloudCAPI_SetCredentials(api,NULL);
// Call EvaluateCredentials to proceed with authorization.
API_EXTERN int  SoundCloudCAPI_EvaluateCredentials(SoundCloudCAPI *api);						// Evaluate credential status
API_EXTERN const char *SoundCloudCAPI_GetUserAuthorizationURL(SoundCloudCAPI *api);			// If user needs to authorize app access, this gives you the URL they must visit
// Authentication status (return code from EvaluateCredentials):
#define SCAuthenticationStatus_ErrorCouldNotRequest		3	// No credential, or a request token was supplied. Could not connect to the server to proceed.
#define SCAuthenticationStatus_UserMustAuthorize		2	// Got a request token from the server, and the user must follow the link to auth. Call GetUserAuthenticationURL.
#define SCAuthenticationStatus_ErrorCouldNotAccess		1	// Tried to connect to the server, assuming user has authed, but could not connect.
#define SCAuthenticationStatus_Authenticated			0	// We have a valid access token.
#define SCAuthenticationStatus_CredentialsHaveChanged	256	// Flag or-ed in to indicate that credientials have changed and should be saved.
#define SCAuthenticationStatus_WillCallback				-1	// This is ALWAYS returned if you specified a callback. The callback will be called asynchronously with the return code.
// Response format:
API_EXTERN void SoundCloudCAPI_SetResponseFormat(SoundCloudCAPI *api,int format);
#define	SCResponseFormatXML		0
#define	SCResponseFormatJSON	1

// API Callback for data-send:
#define SoundCloudCAPICallback_didFinishWithData	0
#define SoundCloudCAPICallback_didFailWithError		1
#define SoundCloudCAPICallback_didReceiveBytes		2
#define SoundCloudCAPICallback_didSendBytes			3
typedef void (*SoundCloudCAPICallback)(SoundCloudCAPI *api,int reason,int errornum,void *data,unsigned long long count,unsigned long long total,void *user);

typedef struct {
	const char *key;		// keyname, such as track[title] or comment[body] or track[asset_data]
	const void *value;		// pointer to the data such as "My Great New Track", "Epic Breakdown" or a PTR to data to upload
	long long value_len;	// the length of the data. if value is a pointer to a string, you can leave this at 0.
	const char *filename;	// pointer to the filename, if there is one. Set this to indicate that you're sending binary data. leave this NULL otherwise.
} SoundCloudCAPI_Parameter;

// API method:
API_EXTERN int SoundCloudCAPI_performMethod(SoundCloudCAPI *api,const char* httpMethod,const char *resource,SoundCloudCAPI_Parameter* parameters,int num_parameters,int *errornum,void **data,unsigned long long *size_recv);
API_EXTERN void SoundCloudCAPI_performMethodWithCallback(SoundCloudCAPI *api,const char* httpMethod,const char *resource,SoundCloudCAPI_Parameter* parameters,int num_params,SoundCloudCAPICallback callback,void *userData);

API_EXTERN SoundCloudCAuthenticationCallback SoundCloudCAPI_DefaultAuthenticationCallback;
API_EXTERN void SoundCloudCAPI_DefaultAuthenticationLoad(SoundCloudCAPI *api);
API_EXTERN void SoundCloudCAPI_DefaultAuthenticationSave(SoundCloudCAPI *api);
API_EXTERN void SoundCloudCAPI_DefaultAuthenticationOpenAuthURL(SoundCloudCAPI *api);

#define SoundCloudCAPI_LogLevel_Silent 0
#define SoundCloudCAPI_LogLevel_Errors 1
#define SoundCloudCAPI_LogLevel_Debug 2
API_EXTERN void SoundCloudCAPI_SetLogLevel(SoundCloudCAPI *api,int level);
	
#ifdef  __cplusplus
}
#endif

#endif