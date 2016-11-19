#ifndef WI_CONCURRENT_JOB_H
#define WI_CONCURRENT_JOB_H

namespace wi {
namespace concurrent {

class job {
public:
  enum status {
    CREATED = 0,
    EXECUTING,
    ENDED
  };
  job() : status_(CREATED) {}
  virtual ~job() {}
  virtual void main() = 0;
protected:
  void start() { status_ = EXECUTING; }
  void end() { status_ = ENDED; }
private:
  status status_;
};

void jobExecutorFn(job* job);

}; // concurrent
}; // wi

#endif // WI_CONCURRENT_JOB_H

