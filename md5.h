/**
 * Copyright (C) 2006-2014 Henning Norén
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, 
 * USA.
 */

#ifndef _MD5_H_
#define _MD5_H_
#include <stdint.h>

void 
md5(const uint8_t *msg, const unsigned int msgLen, uint8_t *digest);

/** init function for md5_50 which chooses a md5_50 optimised for msgLen, 
    if one is available */
void
md5_50_init(const unsigned int msgLen);

/** md5_50 is basically for(i=0; i<50; i++) { md5(msg, msgLen, msg); } */
void
md5_50(uint8_t *msg, const unsigned int msgLen);

#endif /** _MD5_H_ */
