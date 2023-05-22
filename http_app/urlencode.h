/*
 * urlencode.h
 *
 *  Created on: Dec 23, 2021
 *      Author: exeland
 */

#ifndef HTTP_APP_URLENCODE_H_
#define HTTP_APP_URLENCODE_H_

void url_encode(char *working, const char *input);
void url_decode(char *output, const char *input);

#endif /* HTTP_APP_URLENCODE_H_ */
