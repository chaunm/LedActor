/*
 * LedActor.c
 *
 *  Created on: Jun 17, 2016
 *      Author: ChauNM
 */

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <jansson.h>
#include <pthread.h>
#include "actor.h"
#include "led.h"
#include "universal.h"
#include "LedActor.h"
#include "common/ActorParser.h"

#ifdef PI_RUNNING
#include "unistd.h"
#endif

static PACTOR pLedActor;

static void LedActorPublishStateChange(const char* color, char nState)
{
	if (pLedActor == NULL) return;
	json_t* eventJson = json_object();
	json_t* paramsJson = json_object();
	json_t* colorJson = NULL;
	json_t* stateJson = NULL;
	switch (nState)
	{
	case LED_OFF:
		stateJson = json_string("state.off");
		break;
	case LED_ON:
		stateJson = json_string("state.on");
		break;
	case LED_BLINK:
		stateJson = json_string("state.blinking");
		break;
	}
	if (color != NULL)
	{
		colorJson = json_string(color);
		json_object_set(paramsJson, "color", colorJson);
	}
	json_object_set(paramsJson, "state", stateJson);
	json_object_set(eventJson, "params", paramsJson);
	char* eventMessage = json_dumps(eventJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	char* topicName = ActorMakeTopicName(pLedActor->guid, "/:event/state_changed");
	ActorSend(pLedActor, topicName, eventMessage, NULL, FALSE);
	json_decref(colorJson);
	json_decref(stateJson);
	json_decref(paramsJson);
	json_decref(eventJson);
	free(topicName);
	free(eventMessage);
}

static void LedActorOnRequestTurnOn(PVOID pParam)
{
	char* message = (char*)pParam;
	char **znpSplitMessage;
	if (pLedActor == NULL) return;
	json_t* payloadJson = NULL;
	json_t* paramsJson = NULL;
	json_t* colorJson = NULL;
	json_t* responseJson = NULL;
	json_t* statusJson = NULL;
	PACTORHEADER header;
	char* responseTopic;
	char* responseMessage;
	const char* colorMessage;
	znpSplitMessage = ActorSplitMessage(message);
	if (znpSplitMessage == NULL)
		return;
	// parse header to get origin of message
	header = ActorParseHeader(znpSplitMessage[0]);
	if (header == NULL)
	{
		ActorFreeSplitMessage(znpSplitMessage);
		return;
	}
	//parse payload
	payloadJson = json_loads(znpSplitMessage[1], JSON_DECODE_ANY, NULL);
	if (payloadJson == NULL)
	{
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	paramsJson = json_object_get(payloadJson, "params");
	if (paramsJson == NULL)
	{
		json_decref(payloadJson);
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	colorJson = json_object_get(paramsJson, "color");
	if (colorJson == NULL)
	{
		json_decref(paramsJson);
		json_decref(payloadJson);
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	colorMessage = json_string_value(colorJson);
	int result = LedTurnOn(colorMessage);

	//make response package
	responseJson = json_object();
	statusJson = json_object();
	json_t* requestJson = json_loads(znpSplitMessage[1], JSON_DECODE_ANY, NULL);
	json_object_set(responseJson, "request", requestJson);
	json_decref(requestJson);
	json_t* resultJson;
	if (result == 0)
	{
		LedActorPublishStateChange(colorMessage, LED_ON);
		resultJson = json_string("status.success");
	}
	else
	{
		LedActorPublishStateChange(colorMessage, LED_OFF);
		resultJson = json_string("status.failure");
	}
	json_decref(colorJson);
	json_decref(paramsJson);
	json_decref(payloadJson);
	json_object_set(statusJson, "status", resultJson);
	json_decref(resultJson);
	json_object_set(responseJson, "response", statusJson);
	json_decref(statusJson);
	responseMessage = json_dumps(responseJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	responseTopic = ActorMakeTopicName(header->origin, "/:response");
	ActorFreeHeaderStruct(header);
	json_decref(responseJson);
	ActorFreeSplitMessage(znpSplitMessage);
	ActorSend(pLedActor, responseTopic, responseMessage, NULL, FALSE);
	free(responseMessage);
	free(responseTopic);
}

static void LedActorOnRequestTurnOff(PVOID pParam)
{
	char* message = (char*)pParam;
	char **znpSplitMessage;
	if (pLedActor == NULL) return;
	json_t* payloadJson = NULL;
	json_t* paramsJson = NULL;
	json_t* responseJson = NULL;
	json_t* statusJson = NULL;
	PACTORHEADER header;
	char* responseTopic;
	char* responseMessage;
	znpSplitMessage = ActorSplitMessage(message);
	if (znpSplitMessage == NULL)
		return;
	// parse header to get origin of message
	header = ActorParseHeader(znpSplitMessage[0]);
	if (header == NULL)
	{
		ActorFreeSplitMessage(znpSplitMessage);
		return;
	}
	//parse payload
	payloadJson = json_loads(znpSplitMessage[1], JSON_DECODE_ANY, NULL);
	if (payloadJson == NULL)
	{
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	paramsJson = json_object_get(payloadJson, "params");
	if (paramsJson == NULL)
	{
		json_decref(payloadJson);
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	LedTurnOff();
	json_decref(paramsJson);
	json_decref(payloadJson);
	//make response package
	responseJson = json_object();
	statusJson = json_object();
	json_t* requestJson = json_loads(znpSplitMessage[1], JSON_DECODE_ANY, NULL);
	json_object_set(responseJson, "request", requestJson);
	json_decref(requestJson);
	json_t* resultJson;
	LedActorPublishStateChange(NULL, LED_OFF);
	resultJson = json_string("status.success");
	json_object_set(statusJson, "status", resultJson);
	json_decref(resultJson);
	json_object_set(responseJson, "response", statusJson);
	json_decref(statusJson);
	responseMessage = json_dumps(responseJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	responseTopic = ActorMakeTopicName(header->origin, "/:response");
	ActorFreeHeaderStruct(header);
	json_decref(responseJson);
	ActorFreeSplitMessage(znpSplitMessage);
	ActorSend(pLedActor, responseTopic, responseMessage, NULL, FALSE);
	free(responseMessage);
	free(responseTopic);
}

static void LedActorOnRequestBlink(PVOID pParam)
{
	char* message = (char*)pParam;
	char **znpSplitMessage;
	if (pLedActor == NULL) return;
	json_t* payloadJson = NULL;
	json_t* paramsJson = NULL;
	json_t* colorJson = NULL;
	json_t* responseJson = NULL;
	json_t* statusJson = NULL;
	json_t* freqJson = NULL;
	int freq;
	PACTORHEADER header;
	char* responseTopic;
	char* responseMessage;
	const char* colorMessage;
	znpSplitMessage = ActorSplitMessage(message);
	if (znpSplitMessage == NULL)
		return;
	// parse header to get origin of message
	header = ActorParseHeader(znpSplitMessage[0]);
	if (header == NULL)
	{
		ActorFreeSplitMessage(znpSplitMessage);
		return;
	}
	//parse payload
	payloadJson = json_loads(znpSplitMessage[1], JSON_DECODE_ANY, NULL);
	if (payloadJson == NULL)
	{
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	paramsJson = json_object_get(payloadJson, "params");
	if (paramsJson == NULL)
	{
		json_decref(payloadJson);
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	colorJson = json_object_get(paramsJson, "color");
	if (colorJson == NULL)
	{
		json_decref(paramsJson);
		json_decref(payloadJson);
		ActorFreeSplitMessage(znpSplitMessage);
		ActorFreeHeaderStruct(header);
		return;
	}
	colorMessage = json_string_value(colorJson);
	freqJson = json_object_get(paramsJson, "freq");
	if (freqJson == NULL)
		freq = 3;
	else
		freq = json_integer_value(freqJson);

	int result = LedBlink(colorMessage, freq);

	//make response package
	responseJson = json_object();
	statusJson = json_object();
	json_t* requestJson = json_loads(znpSplitMessage[1], JSON_DECODE_ANY, NULL);
	json_object_set(responseJson, "request", requestJson);
	json_decref(requestJson);
	json_t* resultJson;
	if (result == 0)
	{
		LedActorPublishStateChange(colorMessage, LED_BLINK);
		resultJson = json_string("status.success");
	}
	else
	{
		LedActorPublishStateChange(colorMessage, LED_OFF);
		resultJson = json_string("status.failure");
	}
	json_decref(freqJson);
	json_decref(colorJson);
	json_decref(paramsJson);
	json_decref(payloadJson);
	json_object_set(statusJson, "status", resultJson);
	json_decref(resultJson);
	json_object_set(responseJson, "response", statusJson);
	json_decref(statusJson);
	responseMessage = json_dumps(responseJson, JSON_INDENT(4) | JSON_REAL_PRECISION(4));
	responseTopic = ActorMakeTopicName(header->origin, "/:response");
	ActorFreeHeaderStruct(header);
	json_decref(responseJson);
	ActorFreeSplitMessage(znpSplitMessage);
	ActorSend(pLedActor, responseTopic, responseMessage, NULL, FALSE);
	free(responseMessage);
	free(responseTopic);
}

static void LedActorCreate(char* guid, char* psw)
{
	pLedActor = ActorCreate(guid, psw);
	//Register callback to handle request package
	if (pLedActor == NULL)
	{
		printf("Couldn't create actor\n");
		return;
	}
	ActorRegisterCallback(pLedActor, ":request/turn_on", LedActorOnRequestTurnOn, CALLBACK_RETAIN);
	ActorRegisterCallback(pLedActor, ":request/turn_off", LedActorOnRequestTurnOff, CALLBACK_RETAIN);
	ActorRegisterCallback(pLedActor, ":request/blink", LedActorOnRequestBlink, CALLBACK_RETAIN);
}

static void LedActorStart(PLEDACTOROPTION option)
{
	mosquitto_lib_init();
	LedActorCreate(option->guid, option->psw);
	if (pLedActor == NULL)
	{
		mosquitto_lib_cleanup();
		return;
	}
	while(1)
	{
		ActorProcessEvent(pLedActor);
		mosquitto_loop(pLedActor->client, 0, 1);
		usleep(10000);
	}
	mosquitto_disconnect(pLedActor->client);
	mosquitto_destroy(pLedActor->client);
	mosquitto_lib_cleanup();
}

int main(int argc, char* argv[])
{
	/* get option */
	int opt= 0;
	char *token = NULL;
	char *guid = NULL;

	// specific the expected option
	static struct option long_options[] = {
			{"id",      required_argument,  0, 'i' },
			{"token", 	required_argument,  0, 't' },
			{"help", 	no_argument, 		0, 'h' }
	};
	int long_index;
	/* Process option */
	while ((opt = getopt_long(argc, argv,":hi:t:",
			long_options, &long_index )) != -1) {
		switch (opt) {
		case 'h' :
			printf("using: LedActor --<token> --<id>\n"
					"id: guid of the actor\n"
					"token: password of the actor\n");
			exit(EXIT_SUCCESS);
			break;
		case 'i':
			guid = StrDup(optarg);
			break;
		case 't':
			token = StrDup(optarg);
			break;
		case ':':
			if (optopt == 'i')
				printf("invalid option(s), using --help for help\n");
			break;
		default:
			break;
		}
	}
	LedInit();
	/* start actor process */
	LEDACTOROPTION actorOpt;
	actorOpt.guid = guid;
	actorOpt.psw = token;
	LedActorStart(&actorOpt);
	return EXIT_SUCCESS;
}
