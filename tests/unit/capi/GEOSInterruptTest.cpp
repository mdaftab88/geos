// 
// Test Suite for C-API custom allocators

#include <tut.hpp>
// geos
#include <geos_c.h>
// std
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <memory>

namespace tut
{
    //
    // Test Group
    //

    // Common data used in test cases.
    struct test_capiinterrupt_data
    {
        static int alloc_count;
        static int dealloc_count;

        static void notice(const char *fmt, ...)
        {
            std::fprintf( stdout, "NOTICE: ");

            va_list ap;
            va_start(ap, fmt);
            std::vfprintf(stdout, fmt, ap);
            va_end(ap);
        
            std::fprintf(stdout, "\n");
        }

        test_capiinterrupt_data()
        {
        }       

        ~test_capiinterrupt_data()
        {
        }

        static void interruptNow(void *)
        {
          GEOS_interruptRequest();
        }

        static void countCalls(void *arg)
        {
          int* numcalls = reinterpret_cast<int*>(arg);
          ++(*numcalls);
        }

    };

    int test_capiinterrupt_data::alloc_count = 0;
    int test_capiinterrupt_data::dealloc_count = 0;

    typedef test_group<test_capiinterrupt_data> group;
    typedef group::object object;

    group test_capiinterrupt_group("capi::GEOSInterrupt");

    //
    // Test Cases
    //

    /// Test interrupt callback being called
    template<>
    template<>
    void object::test<1>()
    {
        int numcalls = 0;

        initGEOS(notice, notice);

        GEOS_interruptRegisterCallback(countCalls, &numcalls);

        ensure_equals(numcalls, 0);

        GEOSGeometry *geom1 = GEOSGeomFromWKT("LINESTRING(0 0, 1 0)");

        ensure("GEOSGeomFromWKT failed", 0 != geom1);

        GEOSGeometry *geom2 = GEOSBuffer(geom1, 1, 8);

        ensure("GEOSBufferWithStyle failed", 0 != geom2);

        ensure("interrupt callback never called", numcalls > 0);

        GEOSGeom_destroy(geom1);
        GEOSGeom_destroy(geom2);

        GEOS_interruptRegisterCallback(0, 0); /* unregister */


        finishGEOS();
    }

    /// Test interrupt callback being reset by initGEOS
    template<>
    template<>
    void object::test<2>()
    {
        int numcalls = 0;

        GEOS_interruptRegisterCallback(countCalls, &numcalls);

        initGEOS(notice, notice);

        ensure_equals(numcalls, 0);

        GEOSGeometry *geom1 = GEOSGeomFromWKT("LINESTRING(0 0, 1 0)");

        ensure("GEOSGeomFromWKT failed", 0 != geom1);

        GEOSGeometry *geom2 = GEOSBuffer(geom1, 1, 8);

        ensure("GEOSBufferWithStyle failed", 0 != geom2);

        ensure_equals(numcalls, 0);

        GEOSGeom_destroy(geom1);
        GEOSGeom_destroy(geom2);

        GEOS_interruptRegisterCallback(0, 0); /* unregister (shouldn't be needed) */


        finishGEOS();
    }

    /// Test interrupting from callback
    template<>
    template<>
    void object::test<3>()
    {
        initGEOS(notice, notice);

        GEOSGeometry *geom1 = GEOSGeomFromWKT("LINESTRING(0 0, 1 0)");

        ensure("GEOSGeomFromWKT failed", 0 != geom1);

        GEOS_interruptRegisterCallback(interruptNow, 0); 
        bool interrupted = false;
        GEOSGeometry *geom2 = GEOSBuffer(geom1, 1, 8);
        ensure("GEOSBuffer wasn't interrupted", 0 == geom2);
        GEOS_interruptRegisterCallback(0, 0);  /* unregister */

        // TODO: check the actual exception ? (sent to notice() callback)

        GEOSGeom_destroy(geom1);

        finishGEOS();
    }


} // namespace tut

