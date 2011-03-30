# ifdef sun
# define const
# endif

const char *
nullstr(const char *s)
{
    if (!s)
	return "(null)";
    return s;
}

const char *
okfail(int val)
{
    return val ? "[ OK ]" : "[FAIL]";
}
