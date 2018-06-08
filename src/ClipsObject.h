/***************************************************************************
 *  ClipsObject.h - Clips Object Header
 *
 *  Created: 2018-06-05 12:54:11
 *
 *  Copyright QRS
 ****************************************************************************/

#ifndef __ClipsObject_H__
#define __ClipsObject_H__

#include <memory>

/* #define DEBUG_MEMORY */

#ifdef DEBUG_MEMORY
#include <cstdio>
#include <string>
#endif

namespace CLIPS {

class ClipsObject {
public:
    typedef std::shared_ptr<ClipsObject> pointer;

#ifndef DEBUG_MEMORY
    ClipsObject(void *obj = 0) : m_cobj(obj) { }
#else
    ClipsObject(std::string id, void *obj = 0) : __debugID(id), m_cobj(obj)
    {
        printf("-------> create(%p), name(%s)\n", this, __debugID.c_str());
    }
#endif

    ~ClipsObject()
    {
#ifdef DEBUG_MEMORY
        printf("-------> delete(%p), name(%s)\n", this, __debugID.c_str());
#endif
    }

    void* cobj() const { return m_cobj; }

protected:

#ifdef DEBUG_MEMORY
    std::string __debugID;
#endif
    void *m_cobj;

}; /* class ClipsObject */

} /* namespace CLIPS */

#endif /* __ClipsObject_H__ */
