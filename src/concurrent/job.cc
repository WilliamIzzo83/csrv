#include "concurrent/job.h"

void wi::concurrent::jobExecutorFn(job* job) {
  job->main();
}
