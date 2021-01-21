/**
 * @file main.c
 * TCP Socket example
 * @author Ajay Bhargav
 *
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <lib.h>
#include <ril.h>
#include <os_api.h>
#include <network.h>
#include <siwi/sockets.h>

/**
 * Server IP
 */
#define SERVER_IP		"tcpbin.com"
/**
 * Server Port
 */
#define SERVER_PORT		4242

int socket_id;

/**
 * URC Handler
 * @param param1	URC Code
 * @param param2	URC Parameter
 */
static void urc_callback(unsigned int param1, unsigned int param2)
{
	switch (param1) {
	case URC_SYS_INIT_STATE_IND:
		if (param2 == SYS_STATE_SMSOK) {
			/* Ready for SMS */
		}
		break;
	case URC_SIM_CARD_STATE_IND:
		switch (param2) {
		case SIM_STAT_NOT_INSERTED:
			debug(DBG_OFF, "SYSTEM: SIM card not inserted!\n");
			break;
		case SIM_STAT_READY:
			debug(DBG_INFO, "SYSTEM: SIM card Ready!\n");
			break;
		case SIM_STAT_PIN_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PIN required!\n");
			break;
		case SIM_STAT_PUK_REQ:
			debug(DBG_OFF, "SYSTEM: SIM PUK required!\n");
			break;
		case SIM_STAT_NOT_READY:
			debug(DBG_OFF, "SYSTEM: SIM card not recognized!\n");
			break;
		default:
			debug(DBG_OFF, "SYSTEM: SIM ERROR: %d\n", param2);
		}
		break;
	case URC_GSM_NW_STATE_IND:
		debug(DBG_OFF, "SYSTEM: GSM NW State: %d\n", param2);
		break;
	case URC_GPRS_NW_STATE_IND:
		break;
	case URC_CFUN_STATE_IND:
		break;
	case URC_COMING_CALL_IND:
		debug(DBG_OFF, "Incoming voice call from: %s\n", ((ST_ComingCall*)param2)->phoneNumber);
		/* Take action here, Answer/Hang-up */
		break;
	case URC_CALL_STATE_IND:
		switch (param2) {
		case CALL_STATE_BUSY:
			debug(DBG_OFF, "The number you dialed is busy now\n");
			break;
		case CALL_STATE_NO_ANSWER:
			debug(DBG_OFF, "The number you dialed has no answer\n");
			break;
		case CALL_STATE_NO_CARRIER:
			debug(DBG_OFF, "The number you dialed cannot reach\n");
			break;
		case CALL_STATE_NO_DIALTONE:
			debug(DBG_OFF, "No Dial tone\n");
			break;
		default:
			break;
		}
		break;
	case URC_NEW_SMS_IND:
		debug(DBG_OFF, "SMS: New SMS (%d)\n", param2);
		/* Handle New SMS */
		break;
	case URC_MODULE_VOLTAGE_IND:
		debug(DBG_INFO, "VBatt Voltage: %d\n", param2);
		break;
	case URC_ALARM_RING_IND:
		break;
	case URC_FILE_DOWNLOAD_STATUS:
		break;
	case URC_FOTA_STARTED:
		break;
	case URC_FOTA_FINISHED:
		break;
	case URC_FOTA_FAILED:
		break;
	case URC_STKPCI_RSP_IND:
		break;
	default:
		break;
	}
}

static int status_callback(int id, int event, int err)
{
	switch (event)
	{
	case SOCK_EV_CONNECTED:
		printf("Socket connected\n");
		break;
	case SOCK_EV_DISCONNECTED:
		printf("Socket disconnected, Err: %d\n", err);
		break;
	default:
		printf("Socket Event: %d, Err: %d\n", event, err);
		break;
	}
	return 0;
}

static int recv_callback(int sockid, const void *buf, int len)
{
	printf("Data received (%d bytes):\n", len);
	printf("%s", (char *)buf);

	return 0;
}

static int xmit_callback(int sockid, int result, const void *buf, int len, void *arg)
{
	printf("Data send status: %d\n", result);

	return 0;
}

static const struct socket_callback_t sock_cb = {
	.status_callback = status_callback,
	.recv_callback = recv_callback,
	.xmit_callback = xmit_callback,
};

static void socket_init(void)
{
	struct sockopt_t sockopt;

	socket_id = socket_request(SOCK_TYPE_TCP);
	strcpy(sockopt.server_ip, SERVER_IP);
	sockopt.port = SERVER_PORT;
	sockopt.arg = NULL;
	sockopt.autoconnect = TRUE; /* Connect automatically, if disconnected */
	sockopt.handlers = &sock_cb;
	socket_setopt(socket_id, &sockopt);
	socket_open(socket_id);
}

/**
 * Sample Task
 * @param arg	Task Argument
 */
static void socket_task(void *arg)
{
	unsigned char buffer[] = "Hello World\n";

	/* Setup and initialize socket */
	socket_init();

	while (1) {
		/* If socket connected, then send data */
		if (socket_getstatus(socket_id) == SOCK_STA_CONNECTED)
			socket_send(socket_id, buffer, strlen((char *)buffer), 0, NULL);
		sleep(10);
	}
}

/**
 * Application main entry point
 */
int main(int argc, char *argv[])
{
	/*
	 * Initialize library and Setup STDIO
	 */
	siwilib_init("/dev/ttyS0", urc_callback);

	/* Start GPRS service */
	network_gprsenable(TRUE);

	printf("System Ready\n");

	/* Create Application tasks */
	os_create_task(socket_task, NULL, FALSE);

	printf("System Initialization finished\n");

	while (1) {
		/* Main task loop */
		sleep(1);
	}
}
