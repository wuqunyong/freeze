#pragma once

#include <stdint.h>


namespace apie
{
    //  Base class for all objects that participate in inter-thread
    //  communication.

    class object_t
    {
    public:

        object_t (uint32_t tid);
        virtual ~object_t ();

        uint32_t get_tid ();



    private:
        //  Thread ID of the thread the object belongs to.
        uint32_t tid_;
    };

}
