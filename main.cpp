#include "Core/Application.h"

// The entire original int main() (176 lines of connection setup + one
// login call + a 6-way menu switch containing 6 more nested menu loops)
// is now just this. All the state that used to live in globals (con,
// LoggedOnstaff, mchoice/choice reused across blocks) now lives inside
// Database, Session, and Application's locals.
int main()
{
    Application app;
    if (!app.startup())
        return 1;
    app.run();
    return 0;
}