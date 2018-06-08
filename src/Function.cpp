/***************************************************************************
 *   Copyright (C) 2006 by Rick L. Vinyard, Jr.                            *
 *   rvinyard@cs.nmsu.edu                                                  *
 *                                                                         *
 *   This file is part of the clipsmm library.                             *
 *                                                                         *
 *   The clipsmm library is free software; you can redistribute it and/or  *
 *   modify it under the terms of the GNU General Public License           *
 *   version 3 as published by the Free Software Foundation.               *
 *                                                                         *
 *   The clipsmm library is distributed in the hope that it will be        *
 *   useful, but WITHOUT ANY WARRANTY; without even the implied warranty   *
 *   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU   *
 *   General Public License for more details.                              *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this software. If not see <http://www.gnu.org/licenses/>.  *
 ***************************************************************************/
#include "Function.h"

extern "C" {
#include "clips.h"
};

#include "Environment.h"

namespace CLIPS {

Function::Function(Environment &environment, void *cobj)
#ifndef DEBUG_MEMORY
    : ClipsObject(cobj)
#else
    : ClipsObject("Function", cobj)
#endif
    , m_environment(environment)
{

}

Function::pointer Function::create(Environment &environment, void *cobj)
{
    return Function::pointer(new Function(environment, cobj));
}

Function::~Function()
{
}

std::string Function::name()
{
    if (m_cobj)
        return EnvGetDeffunctionName(m_environment.cobj(), m_cobj);
    return std::string();
}

std::string Function::formatted()
{
    const char *format = 0;
    if (m_cobj) {
        format = EnvGetDeffunctionPPForm(m_environment.cobj(), m_cobj);
        if (format)
            return format;
    }
    return std::string();
}

std::string Function::module_name()
{
    if (m_cobj)
        return EnvDeffunctionModule(m_environment.cobj(), m_cobj);
    return std::string();
}

bool Function::is_watched()
{
    if (m_cobj)
        return EnvGetDeffunctionWatch(m_environment.cobj(), m_cobj);
    return false;
}

void Function::set_watch(bool watch)
{
    if (m_cobj)
        EnvSetDeffunctionWatch(m_environment.cobj(), watch, m_cobj);
}

Function::pointer Function::next()
{
    void *nxt;
    if (m_cobj) {
        nxt = EnvGetNextDeffunction(m_environment.cobj(), m_cobj);
        if (nxt)
            return Function::create(m_environment, nxt);
    }
    return Function::pointer();
}

bool Function::is_deletable()
{
    if (m_cobj)
        return EnvIsDeffunctionDeletable(m_environment.cobj(), m_cobj);
    return false;
}

bool Function::undefine()
{
    if (m_cobj)
        return EnvUndeffunction(m_environment.cobj(), m_cobj);
    return false;
}

} /* namespace CLIPS */
