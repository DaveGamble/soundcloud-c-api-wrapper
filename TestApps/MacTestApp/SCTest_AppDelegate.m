/*
 * Copyright 2009 Ullrich Schäfer, Gernot Poetsch for SoundCloud Ltd.
 * 
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 * 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 *
 * For more information and documentation refer to
 * http://soundcloud.com/api
 * 
 */

#import "SCTest_AppDelegate.h"

#import "SCParameterTableDataSource.h"

@interface SCTest_AppDelegate(private)
- (void)commonAwake;
- (void)_registerMyApp;
@end

@implementation SCTest_AppDelegate

#pragma mark Lifecycle

- (void)awakeFromNib;
{
	assert(fetchProgressIndicator != nil);
	assert(httpMethodCombo != nil);
	assert(newParameterAddButton != nil);
	assert(newParameterKeyField != nil);
	assert(newParameterRemoveButton != nil);
	assert(newParameterValueField != nil);
	assert(parametersTableView != nil);
	assert(resourceField != nil);
	assert(responseField != nil);
	assert(sendRequestButton != nil);
	[self commonAwake];
}

- (void)commonAwake;
{
	
	scAPI = SoundCloudCAPI_CreateWithDefaultCallbackAndGetCredentials(kTestAppConsumerKey,kTestAppConsumerSecret,kCallbackURL,kTestAppProductionType);
	SoundCloudCAPI_SetLogLevel(scAPI, SoundCloudCAPI_LogLevel_Debug);
	SoundCloudCAPI_EvaluateCredentials(scAPI);
	
	
	SoundCloudCAPI_SetResponseFormat(scAPI,SCResponseFormatXML);
	
	parametersDataSource = [[SCParameterTableDataSource alloc] init];
	[parametersTableView setDataSource:parametersDataSource];
	
	[self _registerMyApp];
}	

-(void)dealloc;
{
	SoundCloudCAPI_Delete(scAPI);
	[parametersDataSource release];
	[super dealloc];
}

#pragma mark URL handling

- (void)_registerMyApp;
{
	NSAppleEventManager *em = [NSAppleEventManager sharedAppleEventManager];
	[em setEventHandler:self 
			andSelector:@selector(getUrl:withReplyEvent:) 
		  forEventClass:kInternetEventClass 
			 andEventID:kAEGetURL];
	
	NSString *bundleID = [[NSBundle mainBundle] bundleIdentifier];
	OSStatus result = LSSetDefaultHandlerForURLScheme((CFStringRef)@"mycapp", (CFStringRef)bundleID);
	if(result != noErr) {
		NSLog(@"could not register to \"mycapp\" URL scheme");
	}
}

- (void)getUrl:(NSAppleEventDescriptor *)event withReplyEvent:(NSAppleEventDescriptor *)replyEvent
{
	// Get the URL
	NSString *urlStr = [[event paramDescriptorForKeyword:keyDirectObject] stringValue];
	
	if([urlStr hasPrefix:kNSCallbackURL]) {
		NSLog(@"handling oauth callback");
		SoundCloudCAPI_EvaluateCredentials(scAPI);	// Looks like the user AUTHed, let's go!
	}
}

#pragma mark Actions

- (IBAction)addParameter:(id)sender {
	NSString *key = [newParameterKeyField stringValue];
	NSString *value = [newParameterValueField stringValue];
	[parametersDataSource addParameterWithKey:key value:value];
	[parametersTableView reloadData];
}

- (IBAction)removeParameter:(id)sender {
    [parametersDataSource removeParametersAtIndexes:[parametersTableView selectedRowIndexes]];
	[parametersTableView reloadData];
}


- (void)handleCallback:(int)reason withError:(int)error data:(void*)data count:(unsigned long long)count total:(unsigned long long)total
{
	switch (reason)
	{
		case SoundCloudCAPICallback_didFinishWithData:
			[fetchProgressIndicator stopAnimation:nil];
			[postProgress setDoubleValue:0];

			NSString *dataStr = [[NSString alloc] initWithBytes:data length:total encoding:NSASCIIStringEncoding];
			[responseField setString:dataStr];
			[dataStr release];
			break;
		case SoundCloudCAPICallback_didFailWithError:
			[fetchProgressIndicator stopAnimation:nil];
			[postProgress setDoubleValue:0];
			NSLog(@"Request finished with Error: %d\n", error);
			break;
		case SoundCloudCAPICallback_didReceiveBytes:
			NSLog(@"Did Receive Bytes %qu of %qu",count,total);
			break;
		case SoundCloudCAPICallback_didSendBytes:
			NSLog(@"Did send Bytes %qu of %qu", count, total);
			[postProgress setDoubleValue:100 * count / total];
			break;
	}	
}

void DataCallback(SoundCloudCAPI *api,int reason,int errornum,void *data,unsigned long long count,unsigned long long total,void *user)
{
	[(SCTest_AppDelegate*)user handleCallback:reason withError:errornum data:data count:count total:total];
}


SoundCloudCAPI_Parameter *ConstructParameterListFromDictionary(NSDictionary *parameterDictionary,unsigned int count)
{
	SoundCloudCAPI_Parameter *params=(SoundCloudCAPI_Parameter*)malloc(count*sizeof(SoundCloudCAPI_Parameter));
	memset(params,0,count*sizeof(SoundCloudCAPI_Parameter));
	
	int i=0;
	for (NSString *key in [parameterDictionary allKeys]) {
		params[i].key=[key UTF8String];
		params[i].value=[[parameterDictionary objectForKey:key] UTF8String];
		i++;
	}

	return params;
}

- (IBAction)sendRequest:(id)sender {
	[fetchProgressIndicator startAnimation:nil];
	
	const char *resource=[[resourceField stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	const char *method=[[httpMethodCombo stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	unsigned int count=[[parametersDataSource parameterDictionary] count];
	SoundCloudCAPI_Parameter *parameters=ConstructParameterListFromDictionary([parametersDataSource parameterDictionary],count);

	SoundCloudCAPI_performMethodWithCallback(scAPI,method,resource,parameters,count,DataCallback,(void*)self);
	[fetchProgressIndicator startAnimation:nil];

	/* Synch version:
	void *data;unsigned long long datalen;int errnum;
	int res=SoundCloudCAPI_performMethod(scAPI,method,resource,parameters,count,&errnum,&data,&datalen);
	fprintf(stderr,"back again. ret=%d, error=%d, datalen=%d, data=[%s]\n",res,errnum,(int)datalen,data);
	[fetchProgressIndicator stopAnimation:nil];	
	NSString *dataStr = [[NSString alloc] initWithBytes:data length:datalen encoding:NSASCIIStringEncoding];
	[responseField setString:dataStr];
	[dataStr release];
	 */
	
	free(parameters);
}

- (IBAction)postTest:(id)sender;
{
	// sample from http://www.freesound.org/samplesViewSingle.php?id=1375
	NSString *filePath = [[NSBundle mainBundle] pathForResource:@"1375_sleep_90_bpm_nylon2" ofType:@"wav"];

	const char *resource=[[resourceField stringValue] cStringUsingEncoding:NSASCIIStringEncoding];
	unsigned int count=[[parametersDataSource parameterDictionary] count];
	SoundCloudCAPI_Parameter *parameters=ConstructParameterListFromDictionary([parametersDataSource parameterDictionary],count+1);
	parameters[count].key="track[asset_data]";

	FILE *f=fopen([filePath UTF8String],"rb");fseek(f,0,SEEK_END);long flen=ftell(f);fseek(f,0,SEEK_SET);
	char *fdata=malloc(flen);fread(fdata,1,flen,f);fclose(f);
	parameters[count].value=fdata;
	parameters[count].value_len=flen;
	parameters[count].filename="1375_sleep_90_bpm_nylon2";
	
	SoundCloudCAPI_performMethodWithCallback(scAPI,"POST",resource,parameters,count+1,DataCallback,(void*)self);
	free(parameters);free(fdata);

	[fetchProgressIndicator startAnimation:nil];
}

- (IBAction)deleteAllMyTracks:(id)sender;
{	// REMOVED. We're not going to pipe JSON to NSArrays for this.
}


@end
