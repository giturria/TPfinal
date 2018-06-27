/*
 * Application.h
 *
 *      Author: Erich Styger
 */

#ifndef APPLICATION_H_
#define APPLICATION_H_

#include "sapi.h"
/*!
 * \brief Application main routine
 */
void APP_Run(uint8_t temperatura, uint8_t humedadSuelo, uint8_t humedadAmb, uint8_t* temperaturaSet, uint8_t* humedadSet);

#endif /* APPLICATION_H_ */
