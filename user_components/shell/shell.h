/*
 * shell.h
 *
 *  Created on: Dec 23, 2021
 *      Author: exeland
 */

#ifndef _SHELL_H_
#define _SHELL_H_

#ifdef __cplusplus
extern "C" {
#endif

#define SHELL_MAX_ARGUMENTS   (5)


#define TERMINATE_CMD  NULL

typedef void (*term_cmd_t)( int argc,
                            char *argv[],
                            char* out,
                            uint16_t* out_sz );

typedef struct {
  const char   *cmnd;        /**< @brief Command name.       */
  term_cmd_t    func;        /**< @brief Command function.   */
} shell_Command;

bool lock_shell(TickType_t xTicksToWait);
void unlock_shell(void);

bool shell_req(
    char *line,
    char *out_data,
    uint16_t* out_data_sz);

bool shell_init(void);

#ifdef __cplusplus
}
#endif

#endif /* _SHELL_H_ */
