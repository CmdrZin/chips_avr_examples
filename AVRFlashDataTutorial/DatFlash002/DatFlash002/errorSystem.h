/* 
 * The MIT License
 *
 * Copyright 2020 CmdrZin.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sub-license, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * errorSystem.h
 *
 * Created: 11/1/2020 6:26:04 PM
 *  Author: CmdrZin
 */ 


#ifndef ERRORSYSTEM_H_
#define ERRORSYSTEM_H_

typedef enum {ERROR_POWER, ERROR_OVERFLOW, ERROR_TIMEOUT} ERROR_FLAG;

typedef struct
{
	ERROR_FLAG errorFlag;
	char *msg;					// FLASH data has to be fixed length, so use a pointer to a string.
	void (*func)();
} ERROR_RESPONSE;

ERROR_RESPONSE errorHandler(ERROR_FLAG flag);



#endif /* ERRORSYSTEM_H_ */