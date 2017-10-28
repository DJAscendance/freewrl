/****************************************************************************
    This file is part of the FreeWRL/FreeX3D Distribution.

    Copyright 2017 Tom Callaway <tcallawa@redhat.com>

    FreeWRL/FreeX3D is free software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FreeWRL/FreeX3D is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Library General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FreeWRL/FreeX3D.  If not, see <http://www.gnu.org/licenses/>.
****************************************************************************/


#ifndef __FREEWRL_SCENEGRAPH_COMPONENT_GROUPING_H__
#define __FREEWRL_SCENEGRAPH_COMPONENT_GROUPING_H__

void compile_Transform (struct X3D_Transform *node);
void prep_Transform (struct X3D_Transform *node);
void fin_Transform (struct X3D_Transform *node);
void child_Transform (struct X3D_Transform *node);

#endif /* __FREEWRL_SCENEGRAPH_COMPONENT_GROUPING_H__ */