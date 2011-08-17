/*
	Protei — Remote Control and Motor Control
 Copyright (C) 2011  Logan Williams, Gabriella Levine,
                     Qiuyang Zhou, Francois de la Taste
 
 	This file is part of Protei.
 	
 	Protei is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.
 
 Protei is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program (see COPYING).  If not, see <http://www.gnu.org/licenses/>.
 */

/** CONSTANT DEFINITIONS */

// Motors are indexed as follow :
//  0 - bow articulation motor
//  1 - stern articulation motor
//  2 - sail whinch motor
#define   BOW   0
#define   STERN 1
#define   WINCH 2

// this needs to be measured and set manually
const int MAX_MOTOR_ROTATIONS[] = {
  101, 103, 100};

const int GAIN[] = {
  70, 70, 35}; // the proportional gain

// PIN DEFINITIONS
// motor drivers
const int EN_PINS[] 	= {
  42, 44, 36};
const int RPWM_PINS[] 	= {
  5, 4, 8};
const int LPWM_PINS[] 	= {
  6, 7, 9};
// xbee

const int XBEE_TX_PIN = 18;
const int XBEE_RX_PIN = 19;

// motor feedback interrupt pins
const int ROT_PINS[] 		= {
  2, 3, 21};
const int ROT_INTS[] = {
  0, 1, 2};
const int LIMIT_A_PINS[] 	= {
  38, 46, 50};
const int LIMIT_B_PINS[] 	= {
  40, 48}; // the winch does not have a Limit B switch

// the control loop period in ms. 50ms == 20hz
const int CONTROL_LOOP_PERIOD = 50;

// the period of printing debug info over usb
// = usbDebugRate * CONTROL_LOOP_PERIOD = 250ms
const int usbDebugRate = 20;

// the current debug level (higher numbers mean
// that more information is printed)
const int debug = 6;
