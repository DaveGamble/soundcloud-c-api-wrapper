#include "SoundCloudCAPI_Internal.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "oauth.h"
#ifndef WIN
#include <pthread.h>
#else
#include <windows.h>
#endif

// Generate our unique boundary representation.
static char *GenerateBoundary()	{char *boundary=(char*)malloc(28+6+9);sprintf(boundary,"----------------------------sccapi%x",rand());return boundary;}

// Generate a multipart/form-data representation of the parameter list.
static char *ParametersToString(SoundCloudCAPI_Parameter* params,int num_params,size_t *length,const char *boundary)
{
	// first pass, calculating lengths
	int i;
	size_t boundarylen=strlen(boundary)+6;
	size_t len=boundarylen*(num_params+1)+2;
	char **headers;char *param_string,*ptr;
	// if you set value_len=0, we'll call strlen for you. this makes setup for strings very quick.
	for (i=0;i<num_params;i++)	if (!params[i].value_len) params[i].value_len=strlen((const char *)params[i].value);	// correct lengths.

	// second stage; build the headers for each parameter. while doing so, keep a running tally of header length and value length.
	headers=(char**)malloc((num_params+1)*sizeof(char*));	// use +1 so you never malloc(0).
	for (i=0;i<num_params;i++)	// build the header.
	{
		if (params[i].filename)	// binary data!
		{
			headers[i]=(char*)malloc(144+strlen(params[i].key)+strlen(params[i].filename));
			len+=sprintf(headers[i],"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\nContent-Transfer-Encoding: binary\r\nContent-Type: application/octet-stream\r\n\r\n",params[i].key,params[i].filename);
		}
		else
		{
			headers[i]=(char*)malloc(64+strlen(params[i].key));
			len+=sprintf(headers[i],"Content-Disposition: form-data; name=\"%s\"\r\n\r\n",params[i].key);
		}
		len+=(size_t)params[i].value_len;
	}

	// third stage, build the output string:
	param_string=(char*)malloc(len+1);memset(param_string,0,len+1);
	ptr=param_string;		// take a copy to walk with
	for (i=0;i<num_params;i++)
	{
		if (!i) ptr+=sprintf(ptr,"--%s\r\n",boundary);		// start separator
		ptr+=sprintf(ptr,"%s",headers[i]);					// the header
		memcpy(ptr,params[i].value,(size_t)params[i].value_len);	// the data
		ptr+=params[i].value_len;
		ptr+=sprintf(ptr,"\r\n--%s",boundary);				// middle separator at end of data
		if (i==num_params-1) ptr+=sprintf(ptr,"--");		// convert to end separator if needs be
		ptr+=sprintf(ptr,"\r\n");							// newline
		free(headers[i]);									// free these as we go
	}
	free(headers);		// clean up.
	*length=ptr-param_string;
	
	return param_string;	
}

// Construct the strings for the http.
static int ConstructStringsForMethod(SoundCloudCAPI *api,const char* httpMethod,const char *resource,SoundCloudCAPI_Parameter* parameters,int num_params,char **url,char **body,size_t* bodylen,char **header)
{
	char *newurl,*suffix,*tail,*boundary,*headerstring,*pstring,*postargs=0,*sigurl,*signed_url;
	size_t datalen,headlen,taillen,urllen;

	if (api->t_type!=2)	return SoundCloudCAPICallback_didFailWithError;
	
	// Construct the root of the url.
	headlen=strlen(api->apiBaseURL)+strlen(resource);
	if (*resource!='/') headlen++;
	taillen=strlen(httpMethod)+10+4;
	urllen=headlen+taillen;
	newurl=(char*)malloc(urllen+1);newurl[urllen]=0;
	
	// Here's the URL and the resource.
	sprintf(newurl,"%s%s%s",api->apiBaseURL,(*resource=='/')?"":"/",resource);
	if (newurl[headlen-1]=='/') headlen--;
	suffix=newurl+headlen;
	// Add the suffix to request the response format
	tail=suffix+sprintf(suffix,".%s",(api->responseFormat)?"js":"xml");
	// Append the access method
//	tail+=sprintf(tail,"?_method=%s",httpMethod);
	// Unsigned URL is complete.
	
	// Generate a boundary and header for any multipart we have.
	boundary=GenerateBoundary();
	headerstring=(char*)malloc(100+strlen(boundary));	// construct a header string
	sprintf(headerstring,"Content-Type: multipart/form-data; boundary=%s",boundary);
	
	// Construct a binary field containing all the parameter data (files, strings etc).
	pstring=ParametersToString(parameters, num_params,&datalen,boundary);
	if (!datalen) {free(headerstring);headerstring=(char*)malloc(1);*headerstring=0;}	// don't use a multipart header if we don't have actual data.

	// Sign the URL.
	sc_log(api,SoundCloudCAPI_LogLevel_Debug,"Signing url [%s], method [%s], with [%s][%s][%s][%s]\n",newurl,httpMethod,api->consumerKey,api->consumerSecret,api->t_key,api->t_secret);
	sigurl = oauth_sign_url2(newurl, &postargs, OA_HMAC, httpMethod, api->consumerKey, api->consumerSecret, api->t_key, api->t_secret);
	
	// Concatenate the post args onto the url.
	signed_url=(char*)malloc(2+strlen(sigurl)+strlen(postargs));sprintf(signed_url,"%s?%s",sigurl,postargs);
	
	sc_log(api,SoundCloudCAPI_LogLevel_Debug,"Sending Url [%s], header[%s], body[%s]\n",signed_url,headerstring,pstring);
	
	free(newurl);free(boundary);free(sigurl);if(postargs)free(postargs);
	
	(*header)=headerstring;	// SAVE
	(*body)=pstring;
	(*bodylen)=datalen;
	(*url)=signed_url;
	return 0;
}
							   

int SoundCloudCAPI_performMethod(SoundCloudCAPI *api,const char* httpMethod,const char *resource,SoundCloudCAPI_Parameter* parameters,int num_params,int *errornum,void **data,unsigned long long *size_recv)
{
	char *url=0,*body=0,*header=0,*reply;size_t bodylen;
	int err=ConstructStringsForMethod(api,httpMethod,resource,parameters,num_params,&url,&body,&bodylen,&header);

	if (err) return err;	// might have to bail early.
	
	reply = oauth_send_data(url,body,bodylen,*header?header:0,httpMethod);
	*data=reply;
	*size_recv=strlen(reply);

	free(url);
	free(header);
	if (body) free(body);

	return SoundCloudCAPICallback_didFinishWithData;
}

static void oauth_data_callback(void *data,int type,size_t size,size_t total)
{
	SoundCloudCAPIThread *thread=(SoundCloudCAPIThread*)data;
	if (!thread || !thread->callback) return;
	
	thread->callback(thread->api,type?SoundCloudCAPICallback_didSendBytes:SoundCloudCAPICallback_didReceiveBytes,0,0,size,total,thread->callback_data);
}

#ifndef WIN
void* 
#else
DWORD WINAPI
#endif
SoundCloudCAPI_thread(void *data)
{
	// Find the thread data.
	SoundCloudCAPIThread *thread=(SoundCloudCAPIThread*)data;

	if (!thread->callback)	// if no callback, try and send it off.
	{
		if (!thread->error)	oauth_send_data(thread->url,thread->body,thread->bodylen,thread->header,thread->httpMethod);
	}
	else
	{	char *reply; long long len;
		// Proper thread handling with callbacks.
		// If we failed in setup, bail early.
		if (thread->error) {thread->callback(thread->api,SoundCloudCAPICallback_didFailWithError,thread->error,0,0,0,thread->callback_data);return 0;}
		
		reply = oauth_send_data_with_callback(thread->url,thread->body,thread->bodylen,thread->header[0]?thread->header:0,oauth_data_callback,thread,thread->httpMethod);
		len=strlen(reply);
		
		thread->callback(thread->api,SoundCloudCAPICallback_didFinishWithData,0,reply,len,len,thread->callback_data);		
	}

	// Cleanup
	free(thread->url);free(thread->header);if (thread->body) free(thread->body);if (thread->httpMethod) free(thread->httpMethod); free(thread);
	
	return 0;
}

void SoundCloudCAPI_performMethodWithCallback(SoundCloudCAPI *api,const char* httpMethod,const char *resource,SoundCloudCAPI_Parameter* parameters,int num_params,SoundCloudCAPICallback callback,void *userData)
{
	SoundCloudCAPIThread *thread_data=(SoundCloudCAPIThread*)malloc(sizeof(SoundCloudCAPIThread));
	thread_data->api=api;
	thread_data->callback=callback;
	thread_data->callback_data=userData;
	thread_data->httpMethod=sc_strdup(httpMethod);

	thread_data->error=ConstructStringsForMethod(api,httpMethod,resource,parameters,num_params,&thread_data->url,&thread_data->body,&thread_data->bodylen,&thread_data->header);


#ifndef WIN
	pthread_t thread;
	pthread_create(&thread,0,SoundCloudCAPI_thread,thread_data);
#else
	CreateThread (0,0,SoundCloudCAPI_thread,thread_data,0,0);
#endif
}

