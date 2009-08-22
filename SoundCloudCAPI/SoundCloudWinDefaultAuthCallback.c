#include "SoundCloudCAPI.h"
#include "SoundCloudCAPI_Internal.h"
#include <windows.h>

#define default_auth_regkey "SOFTWARE\\SoundCloudCAPI"

void SoundCloudCAPI_DefaultAuthenticationOpenAuthURL(SoundCloudCAPI *api)
{
	ShellExecute(0,"open",SoundCloudCAPI_GetUserAuthorizationURL(api),0,0,SW_SHOWNORMAL);
}

void SoundCloudCAPI_DefaultAuthenticationSave(SoundCloudCAPI *api)
{
	DWORD disp=0;HKEY hkey=0;int status=0;char regkey[256];
	const char *data;
	
	
	data=SoundCloudCAPI_GetCredentials(api);

	sprintf(regkey,"%s\\%s",default_auth_regkey,api->apiBaseURL);

	RegOpenKeyEx(HKEY_CURRENT_USER,regkey,0,KEY_ALL_ACCESS,&hkey);
	if (!hkey)
	{
		status=RegCreateKeyEx(HKEY_CURRENT_USER,regkey,0,0,REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS,NULL,&hkey,&disp);
	}
	if (!status) RegSetValueEx(hkey,api->consumerKey,0,REG_SZ,(const BYTE *)data,strlen(data)+1);
	if (status) sc_log(api,SoundCloudCAPI_LogLevel_Errors,"Failed to save token, with err: %d\n",status);
	RegCloseKey(hkey);
}

void SoundCloudCAPI_DefaultAuthenticationLoad(SoundCloudCAPI *api)
{
	HKEY hkey;DWORD type,l=256;int status;char buffer[256],regkey[256];	// need KEY, need VALUE.

	sprintf(regkey,"%s\\%s",default_auth_regkey,api->apiBaseURL);

	RegOpenKeyEx(HKEY_CURRENT_USER,regkey,0,KEY_QUERY_VALUE,&hkey);
	if (!hkey) return;

	status=RegQueryValueEx(hkey,api->consumerKey,0,&type,(LPBYTE)buffer,&l);
	RegCloseKey(hkey);

	
	if (!status) SoundCloudCAPI_SetCredentials(api,buffer);
}



