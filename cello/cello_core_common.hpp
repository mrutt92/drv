#ifndef CELLO_CORE_COMMON_HPP
#define CELLO_CORE_COMMON_HPP
namespace cello
{
static constexpr int CELLO_TAG = 1;


/**
 * a thread id
 */
struct thread_id_t {
    thread_id_t (){}
    thread_id_t (long pxn, long pod, long core, long thread)
      : pxn(pxn), pod(pod), core(core), thread(thread) {
    }

    bool operator==(const thread_id_t &rhs) const {
        return pxn == rhs.pxn && pod == rhs.pod && core == rhs.core && thread == rhs.thread;
    }

    bool operator!=(const thread_id_t &rhs) const {
        return !(*this == rhs);
    }

    long pxn = 0;
    long pod = 0;
    long core = 0;
    long thread = 0;
};
}
#endif
