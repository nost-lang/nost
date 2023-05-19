
// TODO: the idea of "strict mode" feels a little catch-all. separate out into individual subsystem checks maybe?

#ifdef NOST_ALL_DEBUG_SYSTEMS

#define NOST_GC_STRESS
#define NOST_MEM_TRACK
#define NOST_STRICT_MODE
#define NOST_GREY_TRACK
#define NOST_BLESS_TRACK

#endif
