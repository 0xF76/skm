/**
* @author Jakub Bubak
* @date 30.05.2025
*/

#include "skm_main.h"
#include "shared/tasks.h"
#include "shared/queues.h"

#include <cmsis_os2.h>

void init(void) {
	osKernelInitialize();

	create_tasks();
	create_queues();

	osKernelStart();
}


void error_handler(void) {

}
