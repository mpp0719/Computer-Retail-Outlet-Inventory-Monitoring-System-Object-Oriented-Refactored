#include "Staff/Staff.h"

Session& Session::instance()
{
    static Session session;
    return session;
}