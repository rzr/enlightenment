#ifdef E_TYPEDEFS

#else
# ifndef E_SIGNALS_H
#  define E_SIGNALS_H

EINTERN void e_sigseg_act(int x, siginfo_t *info, void *data);
EINTERN void e_sigill_act(int x, siginfo_t *info, void *data);
EINTERN void e_sigfpe_act(int x, siginfo_t *info, void *data);
EINTERN void e_sigbus_act(int x, siginfo_t *info, void *data);
EINTERN void e_sigabrt_act(int x, siginfo_t *info, void *data);

# endif
#endif
