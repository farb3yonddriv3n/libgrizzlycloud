#include <gc.h>

int hm_log_impl(int priority, struct hm_log_s *log, const char *file, const int line, const char *func, const char *msg, ...)
{
    size_t          len = 0;
    char            out[8192], buf[128];
    time_t          s;
    struct timespec spec;
    long long       ms;
    struct tm       ts;

    assert(log);

    /** only display messages user asked for */
    if(priority > log->priority) {
        return -1;
    }

    //if(log->fd == STDERR_FILENO)
    {
        const char *colour;
        switch(priority) {
            case LOG_EMERG: //< system is unusable
                colour = "\33[1;31;41mLOG_EMERG\33[m";
                break;
            case LOG_ALERT: //< action must be taken immediately
                colour = "\33[1;33;41mLOG_ALERT\33[m";
                break;
            case LOG_CRIT:  //< critical conditions
                colour = "\33[1;31;40mLOG_CRIT\33[m";
                break;
            case LOG_ERR:   //< error conditions
                colour = "\33[1;34;41mLOG_ERR\33[m";
                break;
            case LOG_WARNING: //< warning conditions
                colour = "\33[1;37;43mLOG_WARNING\33[m";
                break;
            case LOG_NOTICE:    //< normal, but significant, condition
                colour = "\33[1;34;47mLOG_NOTICE\33[m";
                break;
            case LOG_INFO:  //< informational message
                colour = "\33[1;37;42mLOG_INFO\33[m";
                break;
            case LOG_DEBUG: //< debug-level message
                colour = "\33[1;32;40mLOG_DEBUG\33[m";
                break;
            case LOG_TRACE: //< memory message
                colour = "\33[1;35;34mLOG_TRACE\33[m";
                break;
            default:
                colour = NULL;
        }
        if(colour) {
            len += snprintf(out, sizeof(out), colour, NULL);
        }
    }

    clock_gettime(CLOCK_REALTIME, &spec);
    s = spec.tv_sec;
    ms = round(spec.tv_nsec / 1.0e6);

    ts = *localtime(&s);
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &ts);

    len += snprintf(out + len, sizeof(out) - len, "[%s.%03lld] ", buf, ms);

    va_list args;
    va_start(args, msg);
    len += vsnprintf(out+len, 3*sizeof(out)/4-len, msg, args);
    va_end(args);
    if(len >= 3*sizeof(out)/4) {
        len += snprintf(out + len, sizeof(out) - len, "...");
    }

    len += snprintf(out+len, sizeof(out)-len, ", %s:%d(%s)\n", file, line, func);

    ssize_t nwritten = write(log->fd, out, len);
    return (nwritten == len) ? 0 : -1;
}

int hm_log_open(struct hm_log_s *l, const char *filename, const int priority)
{
    if(filename != NULL) {
        l->file = fopen(filename, "a");
        if(l->file == NULL) {
            return GC_ERROR;
        }
        l->fd = fileno(l->file);
    } else {
        l->file = stderr;
        l->fd = STDERR_FILENO;
    }

    l->priority = priority;

    return GC_OK;
}

int hm_log_close(struct hm_log_s *l)
{
    if(l->file && l->file != stderr) {
        fclose(l->file);
        return 0;
    } else {
        return -1;
    }
}
