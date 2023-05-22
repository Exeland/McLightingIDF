/*
 * shell.c
 *
 *  Created on: Dec 23, 2021
 *      Author: exeland
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"

#include "esp_system.h"
#include "esp_log.h"

#include "shell.h"
#include "shell_cmd.h"

static const char *SHELL_TAG = "SHELL";

SemaphoreHandle_t shell_mutex = NULL;


static char *parse_arguments(char *str, char **saveptr) {
  char *p;

  if (str != NULL)
    *saveptr = str;

  p = *saveptr;
  if (!p) {
    return NULL;
  }

  /* Skipping white space.*/
  p += strspn(p, " \t");

  if (*p == '"') {
    /* If an argument starts with a double quote then its delimiter is another
       quote.*/
    p++;
    *saveptr = strpbrk(p, "\"");
  }
  else {
    /* The delimiter is white space.*/
    *saveptr = strpbrk(p, " \t");
  }

  /* Replacing the delimiter with a zero.*/
  if (*saveptr != NULL) {
    *(*saveptr)++ = '\0';
  }

  return *p != '\0' ? p : NULL;
}

bool lock_shell(TickType_t xTicksToWait){
  if(shell_mutex){
    if( xSemaphoreTake(shell_mutex, xTicksToWait ) == pdTRUE ) {
      return true;
    }
    else{
      return false;
    }
  }
  else{
    return false;
  }

}

void unlock_shell(void){
  xSemaphoreGive(shell_mutex );
}

bool shell_req(
    char *line,
    char *out_data,
    uint16_t* out_data_sz){
  int n;
  char *lp, *cmd, *tokp;
  char *args[SHELL_MAX_ARGUMENTS + 1];
  bool err_cmnd = false;

  const shell_Command *scp;

  scp = shell_local_commands;

  lp = parse_arguments(line, &tokp);
  cmd = lp;
  n = 0;
  while ((lp = parse_arguments(NULL, &tokp)) != NULL) {
    if (n >= SHELL_MAX_ARGUMENTS) {
      ESP_LOGE(SHELL_TAG, "too many arguments");
      cmd = NULL;
      break;
    }
    args[n++] = lp;
  }
  args[n] = NULL;

  if (cmd != NULL) {
    while (scp->cmnd != NULL) {
      if (strcmp(scp->cmnd, cmd) == 0) {
        scp->func(n, args, out_data, out_data_sz);
        err_cmnd = true;
        break;
      }
      scp++;
    }
  }

  return err_cmnd;
}

bool shell_init(void){
  /* Create a mutex type semaphore. */
  shell_mutex = xSemaphoreCreateMutex();
  return  shell_mutex != NULL ;
}

