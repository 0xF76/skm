/**
* @author Jakub Bubak
* @date 27.05.2025
*/

#include "skm_main.h"
#include "shared/tasks.h"
#include "shared/queues.h"



void init(void) {
  osKernelInitialize();

  create_tasks();
  create_queues();

  osKernelStart();
}

void error_handler(void) {
}

