/*
	Protei — Remote Control and Motor Control
 Copyright (C) 2011  Logan Williams, Gabriella Levine, Qiuyang Zhou
 
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
  300, 300, 300};


const int GAIN[] = {
  8000, 8000, 8000}; // the proportional gain


// PIN DEFINITIONS
// motor drivers
const int EN_PINS[] 	= {
  4, 5, 6};
const int RPWM_PINS[] 	= {
  0, 2, 11};
const int LPWM_PINS[] 	= {
  1, 3, 12};
// xbee
const int XBEE_TX_PIN = 7;
const int XBEE_RX_PIN = 8;

// motor feedback interrupt pins
const int ROT_PINS[] 		= {
  29, 30, 31};
const int LIMIT_A_PINS[] 	= {
  21, 22, 23};
const int LIMIT_B_PINS[] 	= {
  28, 37, 36};

const int CONTROL_LOOP_PERIOD = 50;
const int usbDebugRate = 5;


