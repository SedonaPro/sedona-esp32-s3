/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Allan Stockdill-Mander - initial API and implementation and/or initial documentation
 *******************************************************************************/

#include "MQTTLinux.h"

char expired(Timer* timer)
{
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);		
	return res.tv_sec < 0 || (res.tv_sec == 0 && res.tv_usec <= 0);
}


void countdown_ms(Timer* timer, unsigned int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout / 1000, (timeout % 1000) * 1000};
	timeradd(&now, &interval, &timer->end_time);
}


void countdown(Timer* timer, unsigned int timeout)
{
	struct timeval now;
	gettimeofday(&now, NULL);
	struct timeval interval = {timeout, 0};
	timeradd(&now, &interval, &timer->end_time);
}


int left_ms(Timer* timer)
{
	struct timeval now, res;
	gettimeofday(&now, NULL);
	timersub(&timer->end_time, &now, &res);
	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000;
}

void system_idle(void)
{
	usleep(1000);
}

void InitTimer(Timer* timer)
{
	timer->end_time = (struct timeval){0, 0};
}


int linux_read(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	struct timeval interval = {timeout_ms / 1000, (timeout_ms % 1000) * 1000};
	if (interval.tv_sec < 0 || (interval.tv_sec == 0 && interval.tv_usec <= 0))
	{
		interval.tv_sec = 0;
		interval.tv_usec = 100;
	}

	if (!linux_connection_state(n))
		return -1;

	setsockopt(n->my_socket, SOL_SOCKET, SO_RCVTIMEO, (char *)&interval, sizeof(struct timeval));

	int bytes = 0;
	while (bytes < len)
	{
		size_t req_len = (size_t)(len - bytes);
		int rc = recv(n->my_socket, &buffer[bytes], req_len, 0);
		if (rc == -1)
		{
			if (errno != ENOTCONN && errno != ECONNRESET)
			{
				bytes = -1;
				break;
			}
		}
		else
		{
			if ((rc == 0) && (req_len != 0)) {
				if (!bytes)
					linux_disconnect(n);
				break;
			}
			else
				bytes += rc;
		}
	}
	return bytes;
}


int linux_write(Network* n, unsigned char* buffer, int len, int timeout_ms)
{
	struct timeval tv;

	tv.tv_sec = 0;  /* 30 Secs Timeout */
	tv.tv_usec = timeout_ms * 1000;  // Not init'ing this can cause strange errors

	if (!linux_connection_state(n))
		return -1;

	setsockopt(n->my_socket, SOL_SOCKET, SO_SNDTIMEO, (char *)&tv,sizeof(struct timeval));
	int	rc = send(n->my_socket, buffer, len, MSG_NOSIGNAL);
	if (rc == -1) {
		if ((errno == EPIPE) || (errno == ECONNRESET))
			linux_disconnect(n);
	}
	return rc;
}


void linux_disconnect(Network* n)
{
	close(n->my_socket);
	n->my_socket = -1;
}


bool linux_connection_state(Network* n)
{
	return (n->my_socket != -1);
}

void NewNetwork(Network* n)
{
	n->my_socket = 0;
	n->mqttread = linux_read;
	n->mqttwrite = linux_write;
	n->disconnect = linux_disconnect;
	n->check_state = linux_connection_state;
}

int connect_nonblocking(int sock, struct sockaddr *sa, int sa_len, int timeout)
{
	int flags = 0, error = 0, ret = 0;
	fd_set  rset, wset;
	socklen_t   len = sizeof(error);
	struct timeval  ts;

	ts.tv_sec = timeout;
	ts.tv_usec = 0;

	// clear out descriptor sets for select
	// add socket to the descriptor sets
	FD_ZERO(&rset);
	FD_SET(sock, &rset);
	wset = rset;

	// set socket nonblocking flag
	if( (flags = fcntl(sock, F_GETFL, 0)) < 0)
		return -1;

	if(fcntl(sock, F_SETFL, flags | O_NONBLOCK) < 0)
		return -1;

	// initiate non-blocking connect
	if( (ret = connect(sock, sa, sa_len)) < 0 )
		if (errno != EINPROGRESS)
			return -1;

	// then connect succeeded right away
	if(ret == 0)
		goto done;

	// we are waiting for connect to complete now
	if ((ret = select(sock + 1, &rset, &wset, NULL, (timeout) ? &ts : NULL)) < 0)
		return -1;
	if (ret == 0) {   // we had a timeout
		errno = ETIMEDOUT;
		return -1;
	}

	// we had a positivite return so a descriptor is ready
	if (FD_ISSET(sock, &rset) || FD_ISSET(sock, &wset)) {
		if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
			return -1;
	} else
		return -1;

	// check if we had a socket error
	if (error) {
		errno = error;
		return -1;
	}

done:
	// put socket back in blocking mode
	if (fcntl(sock, F_SETFL, flags) < 0)
		return -1;

	return 0;
}


int ConnectNetwork(Network* n, char* addr, int port)
{
	int type = SOCK_STREAM;
	struct sockaddr_in address;
	int rc = -1;
	sa_family_t family = AF_INET;
	struct addrinfo *result = NULL;
	struct addrinfo hints = {0, AF_UNSPEC, SOCK_STREAM, IPPROTO_TCP, 0, NULL, NULL, NULL};

	if ((rc = getaddrinfo(addr, NULL, &hints, &result)) == 0)
	{
		struct addrinfo* res = result;

		/* prefer ip4 addresses */
		while (res)
		{
			if (res->ai_family == AF_INET)
			{
				result = res;
				break;
			}
			res = res->ai_next;
		}

		if (result->ai_family == AF_INET)
		{
			address.sin_port = htons(port);
			address.sin_family = family = AF_INET;
			address.sin_addr = ((struct sockaddr_in*)(result->ai_addr))->sin_addr;
		}
		else
			rc = -1;

		freeaddrinfo(result);
	}

	if (rc == 0)
	{
		n->my_socket = socket(family, type, 0);
		if (n->my_socket != -1)
		{
			rc = connect_nonblocking(n->my_socket, (struct sockaddr*)&address, sizeof(address), 30);
			if (rc && errno != ETIMEDOUT)
				linux_disconnect(n);
		}
	}

	return rc;
}
