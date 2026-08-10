#include "Staff.h"

Session& Session::instance()
{
    static Session session;
    return session;
}