/*
 * ITransport.h
 *
 *  Created on: Dec 12, 2025
 *      Author: rhinemann
 */

#ifndef COMMUNICATION_ITRANSPORT_H_
#define COMMUNICATION_ITRANSPORT_H_

bool ITransport_init(void);
void ITransport_send(uint8_t *data, uint16_t length);
void ITransport_receive(void);
void ITransport_run(void);

#endif /* COMMUNICATION_ITRANSPORT_H_ */
